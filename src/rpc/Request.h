//
//  Request.hpp
//  PretendToWork
//
//  Created by tqtifnypmb on 18/12/2017.
//  Copyright © 2017 tqtifnypmb. All rights reserved.
//

#pragma once

#include <string>
#include "../3party/json.hpp"

namespace brick
{
    
class Request {
public:
    enum class MethodType {
        // new_view() -> viewId
        // new_view(filePath) -> viewId
        new_view,
        
        // close_view(viewId)
        close_view,
        
        // text(viewId, beginRow, endRow) -> region
        text,
        
        // exit()
        exit,
        
        // for response only
        response,
    };
    
    static Request fromJson(const std::string&);
    Request(size_t id, MethodType method, const nlohmann::json& params);
    Request(size_t id, MethodType method);
    Request(const Request&) = default;
    
    Request response(const nlohmann::json& params) const;
    
    std::string toJson() const;
    
    MethodType method() const {
        return method_;
    }
    
    size_t id() const {
        return id_;
    }
    
    template <class T>
    T getParams(const std::string& key) {
        return params()[key].get<T>();
    }
    
    bool hasParam(const std::string& key) {
        return params()[key].is_null();
    }
    
    nlohmann::json params() const {
        return params_;
    }
    
private:
    size_t id_;
    nlohmann::json params_;
    MethodType method_;
};
    
}   // namespace brick
