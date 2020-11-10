//variant #46 3 2 2

#include <iostream>
#include "ZMQserver.h"
#include "customexceptions.h"
#include <iterator>
#include <chrono>
#include "Request.h"
#include "Response.h"

Tree main_node;

void createNode(Tree* main_node, int id, int parentId) {
    try {
        int result = main_node->insert(id, parentId);
        if (parentId == -1) {
            std::cout << "Ok: " << result << "\n";
        }
    } catch(std::exception& e) {
        std::cout << e.what() << "\n";
    }
}

void removeNode(Tree* main_node, int id) {
    try {
        main_node->remove(id);
    } catch(std::exception& e) {
        std::cout << e.what() << "\n";
    }
}

void insertCommand(Tree* main_node, int id, std::string word, int value) {
    try {
        Request request;
        request.requestType = RequestType::ADD;
        request.varName = word;
        request.value = value;
        main_node->execCommand(request, id);
    } catch(std::exception& e) {
        std::cout << e.what() << "\n";
    }
}

void getCommand(Tree* main_node, int id, std::string word) {
    try {
        Request request;
        request.requestType = RequestType::GET;
        request.varName = word;
        main_node->execCommand(request, id);
    } catch(std::exception& e) {
        std::cout << e.what() << "\n";
    }
}

void pingCommand(Tree* main_node, int id) {
    Request request;
    request.requestType = RequestType::PING;
    try {
     main_node->execCommand(request, id);
    } catch(std::exception& e) {
        std::cout << e.what() << "\n";
    }
}

void mqListener() {
    ZmqSocketServer* mainQueue = new ZmqSocketServer(8080);
    while(true) {
        std::string message = mainQueue->recieve_message();
        Response response(message);
        main_node.resetPingStatusWithLock(atoi(response.id.c_str()), false);

        if (response.responseType == ResponseType::OK) {
            std::cout << "Ok:" << response.id << (response.message.empty() ? "" : (":" + response.message)) << std::endl;
        }
        if (response.responseType == ResponseType::ERROR) {
            std::cout << "Error:" << response.id << ":" << response.message << std::endl;
        }
    }
}

void pingSheduler() {
    while (true) {
        main_node.checkPings();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main() {
    std::thread threadMQ(mqListener);
    std::thread threadPing(pingSheduler);
    while(true) {
        std::string command;
        getline(std::cin, command);
        std::istringstream iss(command);
        std::vector<std::string> splitted((std::istream_iterator<std::string>(iss)), 
                                          std::istream_iterator<std::string>());
        if(splitted.empty()) {
            continue;
        }
        if(splitted[0] == "create") {
            if(splitted.size() != 3) {
                std::cout << "Error: wrong command format\n";
                continue;
            }
            createNode(&main_node, atoi(splitted[1].c_str()), atoi(splitted[2].c_str()));
        }
        if(splitted[0] == "remove") {
            if(splitted.size() != 2) {
                std::cout << "Error: wrong command format\n";
                continue;
            }
            removeNode(&main_node, atoi(splitted[1].c_str()));
        }
        if(splitted[0] == "exec") {
            if(splitted.size() == 3) {
                getCommand(&main_node, atoi(splitted[1].c_str()), splitted[2]);
            } else if(splitted.size() == 4) {
                insertCommand(&main_node, atoi(splitted[1].c_str()), splitted[2], atoi(splitted[3].c_str()));
            } else {
                std::cout << "Error: wrong command format\n";
                continue;
            }
        }
        if(splitted[0] == "ping") {
            if(splitted.size() == 2) {
                pingCommand(&main_node, atoi(splitted[1].c_str()));
            } else {
                std::cout << "Error: wrong command format\n";
                continue;
            }
        }
        if(splitted[0] == "stop") {
            break;
        }
    }
    return 0;
}