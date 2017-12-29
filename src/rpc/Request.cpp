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
    
namespace
{
    
Request::MethodType methodTypeFromString(const std::string& method) {
    if (method == "new_view") {
        return Request::MethodType::new_view;
    } else if (method == "close_view") {
        return Request::MethodType::close_view;
    } else if (method == "text") {
        return Request::MethodType::text;
    } else if (method == "exit") {
        return Request::MethodType::exit;
    } else if (method == "insert") {
        return Request::MethodType::insert;
    } else if (method == "erase") {
        return Request::MethodType::erase;
    } else if (method == "scroll") {
        return Request::MethodType::scroll;
    } else if (method == "select") {
        return Request::MethodType::select;
    } else if (method == "update") {
        return Request::MethodType::update;
    }
    
    throw std::invalid_argument("Unknown rpc method");
}
    
std::string methodTypeToString(Request::MethodType type) {
    switch (type) {
        case Request::MethodType::new_view:
            return "new_view";
            
        case Request::MethodType::close_view:
            return "close_view";
            
        case Request::MethodType::text:
            return "text";
            
        case Request::MethodType::exit:
            return "exit";
            
        case Request::MethodType::insert:
            return "insert";
            
        case Request::MethodType::erase:
            return "erase";
            
        case Request::MethodType::scroll:
            return "scroll";
            
        case Request::MethodType::select:
            return "select";
            
        case Request::MethodType::update:
            return "update";
            
        case Request::MethodType::response:
            return "response";
    }
}
    
}   // namespace
    
Request::Request(size_t id, MethodType method, const nlohmann::json& params)
    : method_(method)
    , params_(params)
    , id_(id) {}
    
Request::Request(size_t id, MethodType method): Request(id, method, nlohmann::json()) {}
    
std::string Request::toJson() const {
    json ret;
    ret["jsonrpc"] = "2.0";
    ret["params"] = params_;
    ret["id"] = id_;
    
    if (method() != MethodType::response) {
        ret["method"] = methodTypeToString(method());
    }
    return ret.dump();
}
    
Request Request::fromJson(const std::string& jstr) {
    auto reqJson = json::parse(jstr);
    auto version = reqJson["jsonrpc"].get<std::string>();
    Expects(version == "2.0");
    
    auto id = reqJson["id"].get<size_t>();
    MethodType method = MethodType::response;
    if (!reqJson["method"].is_null()) {
        auto mStr = reqJson["method"].get<std::string>();
        method = methodTypeFromString(mStr);
    }
    return Request(id, method, reqJson["params"]);
}
  
Request Request::response(const nlohmann::json& params) const {
    return Request(id(), MethodType::response, params);
}
    
}   // namespace brick
