//
//  Request.cpp
//  PretendToWork
//
//  Created by tqtifnypmb on 18/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#include "Request.h"
#include <gsl/gsl>
#include <iostream>

using namespace nlohmann;
using namespace gsl;

namespace brick
{
    
Request::Request(size_t id, MethodType method, const nlohmann::json& params)
    : method_(method)
    , params_(params)
    , id_(id) {}
    
Request::Request(size_t id, MethodType method): Request(id, method, "") {}
    
std::string Request::toJson() const {
    json ret;
    ret["jsonrpc"] = "2.0";
    ret["params"] = params_;
    ret["id"] = id_;
    switch (method()) {
        case MethodType::new_view:
            ret["method"] = "new_view";
            break;
            
        case MethodType::close_view:
            ret["method"] = "close_view";
            break;
            
        case MethodType::text:
            ret["method"] = "text";
            break;
            
        case MethodType::exit:
            ret["method"] = "exit";
            break;
            
        case MethodType::none:
            break;
    }
    return ret.dump();
}
    
Request Request::fromJson(const std::string& jstr) {
    auto reqJson = json::parse(jstr);
    auto version = reqJson["jsonrpc"].get<std::string>();
    Expects(version == "2.0");
    
    auto id = reqJson["id"].get<size_t>();
    
    if (reqJson["method"].is_null()) {
        return Request(id, MethodType::none, reqJson["params"]);
    } else {
        auto method = reqJson["method"].get<std::string>();
        MethodType mType;
        if (method == "new_view") {
            mType = MethodType::new_view;
        } else if (method == "close_view") {
            mType = MethodType::close_view;
        } else if (method == "text") {
            mType = MethodType::text;
        } else if (method == "exit") {
            mType = MethodType::exit;
        } else {
            throw std::invalid_argument("Unknown rpc method");
        }
        
        return Request(id, mType, reqJson["params"]);
    }
}
  
Request Request::response(const nlohmann::json& params) const {
    return Request(id(), MethodType::none, params);
}
    
}   // namespace brick
