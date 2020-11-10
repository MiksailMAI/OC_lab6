#include <iostream>
#include <string>
#include <map>
#include <iterator>
#include <sstream>
#include <vector>
#include <cstring>
#include "zmqhelp.h"
#include <set>
#include "Request.h"
#include "Response.h"
#include <unistd.h>

std::map<std::string, long long> dict;
std::string id;
std::map<int, ZmqSocketClient*> childs;

Response smthtodo(Request& request) {
    Response response;
    response.id = id;
    response.responseType = ResponseType::OK;

    if (request.requestType != RequestType::DELETE && !request.path.empty()) {
        int nodeTo = request.path.at(0);
        request.path.erase(request.path.begin());
        childs[nodeTo]->send_message(request.serialize());
        response.message = "";
        response.responseType = ResponseType::REDIRECTED;
        return response;
    }

    if(request.requestType == RequestType::GET) {
        if(dict.find(request.varName) != dict.end()) {
            response.message = std::to_string(dict[request.varName]);
        } else {
            response.message = "'" + request.varName + "' not found";
        }
    }
    if(request.requestType == RequestType::ADD) {
        dict[request.varName] = request.value;
        response.message = "";
    }
    if(request.requestType == RequestType::PING) {
        response.message = "1";
    }
    if(request.requestType == RequestType::DELETE) {
        if (request.path.size() == 1) {
            int nodeTo = request.path.at(0);
            request.path.erase(request.path.begin());
            childs[nodeTo]->send_message(request.serialize());
            childs.erase(childs.find(nodeTo));
            response.message = "";
        } else if (request.path.empty()){
            for (auto client : childs) {
                client.second->send_message(request.serialize());
            }
            response.message = "break";
        } else {
            int nodeTo = request.path.at(0);
            request.path.erase(request.path.begin());
            childs[nodeTo]->send_message(request.serialize());
            response.message = "";
        }
    }
    if (request.requestType == RequestType::CREATE) {
        if (request.path.size() == 0) {
            int port = request.portForCreate;
            ZmqSocketClient* client = new ZmqSocketClient(port);
            childs.insert(std::make_pair(request.toCreateNodeId, client));
            int pid = fork();
            if (pid == 0) {
                execl("treenode", std::to_string(port).c_str(), std::to_string(request.toCreateNodeId).c_str(), NULL);
            } else if(pid > 0) {
                response.message = std::to_string(pid);
            } else {
                response.responseType = ResponseType::ERROR;
                response.message = "Fork exception";
            }
        }
    }
    return response;
}

int main(int argc, char *argv[]) {
    std::cout << "STARTING NODE on port: " << argv[0] << "\n";
    id = argv[1];
    ZmqSocketServer* server = new ZmqSocketServer(atoi(argv[0]));
    ZmqSocketClient* client = new ZmqSocketClient(8080);
    while(true) {
        Request request(server->recieve_message());
        Response result = smthtodo(request);
        client->send_message(result.serialize());
        if(result.message == "break") break;
    }
    std::cout << "Stopping server on port " << id << std::endl;
    return 0;
}