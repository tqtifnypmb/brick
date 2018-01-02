#include "client/Client.h"

#include "../../rpc/Request.h"
#include <iostream>
#include <sstream>

#include <unistd.h>
#include <string>

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
    auto raw = req.toJson();
    std::cout<<"Raw: "<<raw<<std::endl;
    client.write(raw);
    end();
}

int currentView = -1;

std::string readUserInput() {
    char buf[128];
    auto nbytes = read(STDIN_FILENO, buf, 128);
    return std::string(buf, nbytes);
}

void handle_user_input(Client& client) {
    std::string input = readUserInput();
    auto endParams = input.find_last_of(' ');
    char c;
    
    if (endParams == std::string::npos) {
        c = input[0];
        input.clear();
    } else {
        c = input[input.size() - 2];
        input.erase(endParams, input.size());
    }
    
    std::cout<<"Command: "<<c<<std::endl;
    std::cout<<"Params: "<<input<<std::endl;
    
    if (c == 'Q') {
        sendReq(client, Request::MethodType::exit, "", "Exit Req");
    }
    
    else if (c == 'I') {
        auto params = nlohmann::json::object();
        params["viewId"] = currentView;
        params["bytes"] = input;
        sendReq(client, Request::MethodType::insert, params, "Insert Req");
    }
    
    else if (c == 'V') {
        if (!input.empty()) {
            auto params = nlohmann::json::object();
            params["filePath"] = input;
            sendReq(client, Request::MethodType::new_view, params, "New View Req");
        } else {
            sendReq(client, Request::MethodType::new_view, nlohmann::json(), "New View Req");
        }
    }
    
    else if (c == 'E') {
        auto params = nlohmann::json::object();
        params["viewId"] = currentView;
        sendReq(client, Request::MethodType::erase, params, "Erase Req");
    }
    
    else if (c == 'S') {
        auto sstm = std::stringstream(input);
        int begRow = 0;
        int endRow = 0;
        sstm >> begRow;
        sstm >> endRow;
        
        auto params = nlohmann::json::object();
        params["viewId"] = currentView;
        params["range"] = std::vector<int> { begRow, endRow };
        sendReq(client, Request::MethodType::scroll, params, "Scroll Req");
    }
    
    else if (c == 'M') {
        auto sstm = std::stringstream(input);
        int pos = 0;
        int len = 0;
        sstm >> pos;
        sstm >> len;
        
        auto params = nlohmann::json::object();
        params["viewId"] = currentView;
        params["range"] = std::vector<int> { pos, len };
        sendReq(client, Request::MethodType::select, params, "Scroll Req");
    }
    
    else if (c == 'T') {
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
    }
    
    else if (c == 'C') {
        auto params = nlohmann::json::object();
        params["viewId"] = currentView;
        sendReq(client, Request::MethodType::close_view, params, "Close View Req");
        client.stop();
    }
    
    else {
        throw std::domain_error("Invalid input");
    }
}

void handler_core_input(Client& client) {
    auto respJson = client.read();
    std::cout<<"Raw: "<<respJson<<std::endl;
    auto resp = Request::fromJson(respJson);
    std::cout<<resp.params()<<std::endl;
    
    if (currentView == -1 && resp.hasParam("viewId")) {
        currentView = resp.getParams<int>("viewId");
    }
    
    if (resp.method() == Request::MethodType::response) {
        
    } else if (resp.method() == Request::MethodType::update) {
        
    } else {
        throw std::invalid_argument("Unknown resp sent from core");
    }
    
    end();
}

int main() {
    Client client("127.0.0.1", 9999, handle_user_input,handler_core_input);
    return client.loop();
}
