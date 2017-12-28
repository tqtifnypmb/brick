#include "client/Client.h"

#include "../../rpc/Request.h"
#include <iostream>
#include <sstream>

using namespace brick;

void begin(const std::string& method) {
    std::cout<<"==="<<method<<"==="<<std::endl;
}

void end() {
    std::cout<<"========================"<<std::endl;
}

void sendReq(Client& client, Request::MethodType method, const nlohmann::json& params, const std::string& log) {
    auto reqId = client.requestId();
    auto req = Request(reqId, method, params);
    begin(log);
    std::cout<<"Req Id: "<<reqId<<std::endl;
    auto raw = req.toJson();
    std::cout<<"Raw: "<<raw<<std::endl;
    client.write(raw);
    end();
}

template <class T>
T readResp(Client& client, const std::string& log, const std::string& logParamName) {
    begin(log);
    auto resp = client.read();
    std::cout<<"Raw: "<<resp<<std::endl;
    
    auto respJson = Request::fromJson(resp);
    
    T param = respJson.getParams<T>(logParamName);
    auto respId = respJson.id();
    std::cout<<logParamName<<": "<<param<<std::endl;
    std::cout<<"Resp Id: "<<respId<<std::endl;
    end();
    
    return param;
}

int main() {
    Client client("127.0.0.1", 9999);
    
    int currentView = -1;
    std::string input;
    for (;;) {
        auto c = getchar();
        if (c == 'Q') {
            sendReq(client, Request::MethodType::exit, "", "Exit Req");
            break;
        }
        
        if (c == 'I') {
            auto params = nlohmann::json::object();
            params["viewId"] = currentView;
            params["bytes"] = input;
            sendReq(client, Request::MethodType::insert, params, "Insert Req");
            
            input.clear();
            getchar();
            continue;
        }
        
        if (c == 'V') {
            sendReq(client, Request::MethodType::new_view, nlohmann::json(), "New View Req");
            currentView = readResp<int>(client, "New View Response", "viewId");
            
            input.clear();
            getchar();
            continue;
        }
        
        if (c == 'E') {
            auto params = nlohmann::json::object();
            params["viewId"] = currentView;
            sendReq(client, Request::MethodType::erase, params, "Erase Req");
            
            input.clear();
            getchar();
            continue;
        }
        
        if (c == 'S') {
            auto sstm = std::stringstream(input);
            int begRow = 0;
            int endRow = 0;
            sstm >> begRow;
            sstm >> endRow;
            
            auto params = nlohmann::json::object();
            params["viewId"] = currentView;
            params["range"] = std::vector<int> { begRow, endRow };
            sendReq(client, Request::MethodType::scroll, params, "Scroll Req");
            
            input.clear();
            getchar();
            continue;
        }
        
        if (c == 'M') {
            auto sstm = std::stringstream(input);
            int pos = 0;
            int len = 0;
            sstm >> pos;
            sstm >> len;
            
            auto params = nlohmann::json::object();
            params["viewId"] = currentView;
            params["range"] = std::vector<int> { pos, len };
            sendReq(client, Request::MethodType::select, params, "Scroll Req");
            
            input.clear();
            getchar();
            continue;
        }
        
        if (c == 'T') {
            if (input.empty()) {
                auto params = nlohmann::json::object();
                params["viewId"] = currentView;
                sendReq(client, Request::MethodType::text, params, "Text Req");
            } else {
                auto sstm = std::stringstream(input);
                int begRow = 0;
                int endRow = 0;
                sstm >> begRow;
                sstm >> endRow;
                auto params = nlohmann::json::object();
                params["viewId"] = currentView;
                params["range"] = std::vector<int> { begRow, endRow };
                sendReq(client, Request::MethodType::text, params, "Text Req");
            }
            
            auto text = readResp<nlohmann::json>(client, "Text Resp", "region");
            std::cout<<"===== Region ====="<<std::endl;
            for (const auto& line : text) {
                std::cout<<line["line"].get<int>()<<" "<<line["bytes"].get<std::string>()<<std::endl;
            }
            std::cout<<"=================="<<std::endl;
            input.clear();
            getchar();
            continue;
        }
        
        if (c == 'C') {
            auto params = nlohmann::json::object();
            params["viewId"] = currentView;
            sendReq(client, Request::MethodType::close_view, params, "Close View Req");
            input.clear();
            getchar();
            continue;
        }
        
        input.append(1, static_cast<char>(c));
    }
    
    return 0;
}
