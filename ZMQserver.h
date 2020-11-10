#pragma once


#ifndef ZMQSERVER_HPP
#define ZMQSERVER_HPP

#include <unistd.h>
#include <fcntl.h>

#include "zmqhelp.h"
#include "customexceptions.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <queue>
#include "MySemaphore.h"
#include <unordered_set>
#include "Request.h"
#include "ping.h"


struct TreeNode {
    std::unordered_map<int, TreeNode*> child;
    int id;
    TreeNode(const int in) {
        id = in;
    }
};

class Tree {
private:
    TreeNode* root;
    std::unordered_map<int, ZmqSocketClient*> sockets;
    std::unordered_map<int, PingMessage> pings;
    binsem pingSem;

    static int port;
    static int maxTicks;

public:

    Tree() {
        root = new TreeNode(-1);
    }
    
    int insert(int id, int parentId) {
        TreeNode* node = new TreeNode(id);
        if (parentId == -1) {
            if (root->child.find(id) == root->child.end()) {
                root->child.insert(std::make_pair(id, node));
                pingSem.lock();
                pings.insert(std::make_pair(id, PingMessage()));
                pingSem.unlock();
                return createChildProcess(node);
            } else {
                throw AlreadyExistsException();
            }
        }

        std::vector<int> path;
        if (getPath(root, id, path)) {
            throw AlreadyExistsException();
        }
        
        path.clear();
        getPath(root, parentId, path);

        if (path.empty()) {
            throw ParentNotFoundException();
        }

        //erasing root node
        path.erase(path.begin());

        TreeNode* ptr = root;
        for (int i : path) {
            ptr = ptr->child[i];
        }
        ptr->child.insert(std::make_pair(id, node));
        pingSem.lock();
        pings.insert(std::make_pair(id, PingMessage()));
        pingSem.unlock();

        Request request;
        request.requestType = RequestType::CREATE;
        request.toCreateNodeId = id;
        request.portForCreate = port++;
        execCommand(request, parentId);
        return 0;
    }

    void remove(int id) {
        Request request;
        request.requestType = RequestType::DELETE;
        if (id == -1) {
            for (auto it : root->child) {
                execCommand(request, it.first);
            }
            root->child.clear();
            return;
        }
        execCommand(request, id);
        std::vector<int> path;
        getPath(root, id, path);
        path.erase(path.begin());
        path.pop_back();
        TreeNode* ptr = root;
        for (int i : path) {
            ptr = ptr->child[i];
        }
        std::queue<TreeNode*> nodes;
        nodes.push(ptr->child[id]);
        pingSem.lock();
        while(!nodes.empty()) {
            TreeNode* current = nodes.front();
            nodes.pop();
            for (auto it : current->child) {
                nodes.push(it.second);
            }
            pings.erase(pings.find(current->id));
        }
        pingSem.unlock();

        ptr->child.erase(ptr->child.find(id));
    }

    void execCommand(Request& request, int id) {
        getPath(root, id, request.path);
        if (request.path.empty()) {
            throw NotExistsException(id);
        }
        request.path.erase(request.path.begin());
        pingSem.lock();
        for (int i : request.path) {
            resetPingStatus(i, true);
        }
        pingSem.unlock();
        int nodeTo = request.path.at(0);
        request.path.erase(request.path.begin());
        sockets[nodeTo]->send_message(request.serialize());
    }

    void resetPingStatusWithLock(int id, bool status) {
        pingSem.lock();
        resetPingStatus(id, status);
        pingSem.unlock();
    }
    
    void checkPings() {
        pingSem.lock();
        for (auto &it : pings) {
            if (it.second.isWaiting) {
                if (it.second.tick >= maxTicks) {
                    printf("Ping:%d:0\n", it.first);
                    it.second.isWaiting = false;
                    it.second.tick = 0;
                } else {
                    it.second.tick++;
                }
            }
        }
        pingSem.unlock();
    }

private:
    void resetPingStatus(int id, bool status) {
        auto it = pings.find(id);
        if (it != pings.end()) {
            it->second.isWaiting = status;
            it->second.tick = 0;
        }
    }

    bool getPath(TreeNode* node, int id, std::vector<int>& path) const {
        if (node == nullptr) {
            return false;
        }
        if (node->id == id) {
            path.push_back(node->id);
            return true;
        }
        path.push_back(node->id);
        for (auto elem : node->child) {
            if (getPath(elem.second, id, path)) {
                return true;
            }
        }
        path.pop_back();
        return false;
    }

    int createChildProcess(TreeNode* in) {
        int pid = fork();
        if(pid == 0) {
            execl("treenode", std::to_string(port).c_str(), std::to_string(in->id).c_str(), NULL);
        } else if(pid > 0) {
            ZmqSocketClient* socket = new ZmqSocketClient(port);
            sockets[in->id] = socket;
            port++;
            return pid;
        }
        throw ForkException();
    }
};

int Tree::port = 8081;
int Tree::maxTicks = 5;

#endif