#pragma once

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <sstream>
#include <vector>

enum RequestType {
    CREATE,
    ADD,
    GET,
    PING,
    DELETE
};

class Request { // implements Serializable
public:
    RequestType requestType;
    long long value;
    std::string varName;
    std::vector<int> path;
    int toCreateNodeId;
    int portForCreate;

    std::string serialize() {
        std::string result = std::to_string(requestType);

        if (requestType == RequestType::ADD) {
            result = result + " " + varName + " " + std::to_string(value);
        }

        if (requestType == RequestType::GET) {
            result = result + " " + varName;
        }

        if (requestType == RequestType::CREATE) {
            result = result + " " + std::to_string(toCreateNodeId) + " " + std::to_string(portForCreate);
        }

        if (requestType == RequestType::PING) {
            //dont do anything
        }
        if (requestType == RequestType::DELETE) {
            // dont do anything
        }

        result = result + " " + std::to_string(path.size());
        for (int i : path) {
            result = result + " " + std::to_string(i);
        }

        return result;
    }

    Request (std::string in) {
        Request request = deserialize(in);
        requestType = request.requestType;
        value = request.value;
        varName = request.varName;
        path = request.path;
        toCreateNodeId = request.toCreateNodeId;
        portForCreate = request.portForCreate;
    }

    Request () {

    }

    static Request deserialize(std::string in) {
        Request request;
        std::istringstream stream(in);
        int requestType;
        stream >> requestType;
        request.requestType = RequestType(requestType);
        if (requestType == RequestType::ADD) {
            stream >> request.varName >> request.value;
        }

        if (requestType == RequestType::GET) {
            stream >> request.varName;
        }

        if (requestType == RequestType::CREATE) {
            stream >> request.toCreateNodeId >> request.portForCreate;
        }

        if (requestType == RequestType::PING) {
            //dont do anything
        }
        if (requestType == RequestType::DELETE) {
            // dont do anything
        }
        int size = 0;
        std::vector<int> ids;
        stream >> size;
        for (int i = 0; i < size; ++i) {
            int id;
            stream >> id;
            ids.push_back(id);
        }
        request.path = ids;

        return request;
    }

};

#endif