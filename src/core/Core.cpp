//
//  Core.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 22/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Core.h"
#include "../view/View.h"
#include "../editor/Editor.h"
#include "../converter/Converter.h"

#include <gsl/gsl>

#include <iostream>

using namespace gsl;

namespace brick
{

Core::Core(const char* ip, int port)
    : nextReqId_(0)
    , nextViewId_(0) {
    rpc_ = std::make_unique<Rpc>(ip, port, std::bind(&Core::handleReq, this, std::placeholders::_1, std::placeholders::_2));
}
    
void Core::handleReq(Rpc::RpcPeer* peer, Request req) {
    std::cout<<"==== Begin handling request ====="<<std::endl;
    std::cout<<req.toJson()<<std::endl;

    size_t viewId = 0;
    View* view = nullptr;
    if (req.method() != Request::MethodType::new_view) {
        viewId = req.getParams<size_t>("viewId");
        view = viewWithId(viewId);
        if (view == nullptr) {
            std::cerr<<"View("<<viewId<<") not found"<<std::endl;
            return;
        }
    }
    
    switch (req.method()) {
        case Request::MethodType::new_view: {
            auto viewId = nextViewId_++;
            Expects(viewWithId(viewId) == nullptr);
            
            // Must happend before new view is constructed
            // since new view may be sync with parent
            peersMap_[viewId] = peer;
            
            auto updateCb = std::bind(&Core::updateView, this, std::placeholders::_1, std::placeholders::_2);
            if (!req.hasParam("filePath")) {
                auto view = std::make_unique<View>(viewId, updateCb);
                viewsMap_[viewId] = std::move(view);
            } else {
                auto filePath = req.getParams<std::string>("filePath");
                auto parent = viewWithFilePath(filePath);
                if (parent != nullptr) {
                    auto view = std::make_unique<View>(viewId, parent, updateCb);
                    viewsMap_[viewId] = std::move(view);
                } else {
                    auto view = std::make_unique<View>(viewId, filePath, updateCb);
                    viewsMap_[viewId] = std::move(view);
                }
            }
            auto params = nlohmann::json::object();
            params["viewId"] = viewId;
            auto resp = req.response(params);
            sendResp(peer, resp);
            break;
        }
            
        case Request::MethodType::close_view: {
            rpc_->close(peer);
            peersMap_.erase(viewId);
            viewsMap_.erase(viewId);
            break;
        }
            
        case Request::MethodType::text: {
            decltype(view->region<ASCIIConverter>()) text;
            if (req.hasParam("range")) {
                auto range = req.getParams<nlohmann::json>("range");
                auto beginRow = range[0].get<int>();
                auto endRow = range[1].get<int>();
                if (endRow <= beginRow) {
                    throw std::invalid_argument("endRow <= begRow");
                }
                
                text = view->region<ASCIIConverter>(beginRow, endRow);
            } else {
                text = view->region<ASCIIConverter>();
            }

            auto params = nlohmann::json::object();
            if (!text.empty()) {
                for (const auto& line : text) {
                    auto region = nlohmann::json::object();
                    region["line"] = line.first;
                    region["bytes"] = line.second;
                    
                    params["region"].push_back(region);
                }
            } else {
                params["region"] = nlohmann::json::array();
            }
            
            auto resp = req.response(params);
            sendResp(peer, resp);
            break;
        }
            
        case Request::MethodType::exit: {
            rpc_->close();
            break;
        }
            
        case Request::MethodType::insert: {
            auto bytes = req.getParams<std::string>("bytes");
            view->insert<ASCIIConverter>(gsl::make_span(bytes.c_str(), bytes.length()));
            break;
        }
            
        case Request::MethodType::erase: {
            view->erase();
            break;
        }
            
        case Request::MethodType::scroll: {
            auto range = req.getParams<nlohmann::json>("range");
            auto beginRow = range[0].get<int>();
            auto endRow = range[1].get<int>();
            view->scroll(beginRow, endRow);
            break;
        }
            
        case Request::MethodType::select: {
            auto range = req.getParams<nlohmann::json>("range");
            auto pos = range[0].get<int>();
            auto len = range[1].get<int>();
            
            view->select(Range(pos, len));
            break;
        }
         
        case Request::MethodType::save: {
            if (!req.hasParam("filePath")) {
                view->save();
            } else {
                const auto& filePath = req.getParams<std::string>("filePath");
                view->save(filePath);
            }
            break;
        }
            
        case Request::MethodType::update: {
            throw std::invalid_argument("Update request sent to core");
            break;
        }
            
        case Request::MethodType::response: {
            throw std::invalid_argument("Response sent to core");
            break;
        }
    }
    std::cout<<"==== End handling request ====="<<std::endl;
}
    
void Core::sendResp(Rpc::RpcPeer* client, Request req) {
    rpc_->send(client, req.toJson());
}
    
void Core::updateView(View* view, const Editor::DeltaList& deltas) {
    auto peer = portForView(view->viewId());
    Expects(peer != nullptr);
    
    auto params = nlohmann::json::array();
    for (const auto& delta : deltas) {
        const auto& [rev, begRow, endRow] = delta;
        auto subParams = nlohmann::json::object();
        subParams["range"].push_back(rev.range().location);
        subParams["range"].push_back(rev.range().length);
        subParams["begRow"] = begRow;
        subParams["endRow"] = endRow;
        
        switch (rev.op()) {
            case Revision::Operation::erase:
                subParams["op"] = "erase";
                break;
                
            case Revision::Operation::insert:
                subParams["text"] = ASCIIConverter::decode(rev.cplist());
                subParams["op"] = "insert";
                break;
        }
        params.push_back(subParams);
    }
    auto req = Request(nextReqId_++, Request::MethodType::update, params);
    sendResp(peer, req);
}
   
View* Core::viewWithId(size_t viewId) {
    auto iter = viewsMap_.find(viewId);
    if (iter != viewsMap_.end()) {
        return iter->second.get();
    } else {
        return nullptr;
    }
}
    
View* Core::viewWithFilePath(const std::string& filePath) {
    std::vector<View*> views;
    for (auto& v : viewsMap_) {
        if (v.second->filePath() == filePath) {
            views.push_back(v.second.get());
        }
    }
    
    if (views.empty()) {
        return nullptr;
    } else {
        std::sort(views.begin(), views.end(), [](const auto& l, const auto& r) { return l->viewId() < r->viewId(); });
        return views.front();
    }
}
    
Rpc::RpcPeer* Core::portForView(size_t viewId) {
    auto iter = peersMap_.find(viewId);
    if (iter != peersMap_.end()) {
        return iter->second;
    } else {
        return nullptr;
    }
}
    
int Core::run() {
    try {
        rpc_->loop();
    } catch (const std::exception& exp) {
        std::cerr<<exp.what()<<std::endl;
        return -1;
    }
    
    return 0;
}

}   // namespace brick
