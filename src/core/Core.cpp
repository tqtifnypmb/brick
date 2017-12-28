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
    : nextViewId_(0) {
    rpc_ = std::make_unique<Rpc>(ip, port, std::bind(&Core::handleReq, this, std::placeholders::_1, std::placeholders::_2));
}
    
void Core::handleReq(Rpc::RpcPeer* peer, Request req) {
    switch (req.method()) {
        case Request::MethodType::new_view: {
            auto viewId = nextViewId_++;
            Expects(viewWithId(viewId) == nullptr);
            
            auto params = nlohmann::json::object();
            params["viewId"] = viewId;
            auto resp = req.response(params);
            sendResp(peer, resp);
            
            if (!req.hasParam("filePath")) {
                auto view = std::make_unique<View>(viewId);
                viewsMap_[viewId] = std::move(view);
            } else {
                auto filePath = req.getParams<std::string>("filePath");
                auto view = std::make_unique<View>(viewId, filePath);
                viewsMap_[viewId] = std::move(view);
            }
            peersMap_[viewId] = peer;
            break;
        }
            
        case Request::MethodType::close_view: {
            auto viewId = req.getParams<size_t>("viewId");
            Expects(viewWithId(viewId) != nullptr);
            
            peersMap_.erase(viewId);
            viewsMap_.erase(viewId);
            break;
        }
            
        case Request::MethodType::text: {
            auto viewId = req.getParams<size_t>("viewId");
            auto peer = portForView(viewId);
            Expects(peer != nullptr);
            auto view = viewWithId(viewId);
            Expects(view != nullptr);
            
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
            auto viewId = req.getParams<size_t>("viewId");
            auto bytes = req.getParams<std::string>("bytes");
            
            auto view = viewWithId(viewId);
            Expects(view != nullptr);
            
            view->insert<ASCIIConverter>(gsl::make_span(bytes.c_str(), bytes.length()));
            break;
        }
            
        case Request::MethodType::erase: {
            auto viewId = req.getParams<size_t>("viewId");
            
            auto view = viewWithId(viewId);
            Expects(view != nullptr);
            
            view->erase();
            break;
        }
            
        case Request::MethodType::scroll: {
            auto viewId = req.getParams<size_t>("viewId");
            auto range = req.getParams<nlohmann::json>("range");
            auto beginRow = range[0].get<int>();
            auto endRow = range[1].get<int>();
            
            auto view = viewWithId(viewId);
            Expects(view != nullptr);
            
            view->scroll(beginRow, endRow);
            break;
        }
            
        case Request::MethodType::select: {
            auto viewId = req.getParams<size_t>("viewId");
            auto range = req.getParams<nlohmann::json>("range");
            auto pos = range[0].get<int>();
            auto len = range[1].get<int>();
            auto view = viewWithId(viewId);
            Expects(view != nullptr);
            
            view->select(Range(pos, len));
            break;
        }
            
        case Request::MethodType::response: {
            throw std::invalid_argument("Response sent to server");
            break;
        }
    }
}
    
void Core::sendResp(Rpc::RpcPeer* client, Request req) {
    rpc_->send(client, req.toJson());
}
   
View* Core::viewWithId(size_t viewId) {
    auto iter = viewsMap_.find(viewId);
    if (iter != viewsMap_.end()) {
        return iter->second.get();
    } else {
        return nullptr;
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
