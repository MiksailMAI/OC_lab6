#pragma once

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <sstream>
#include <vector>

enum ResponseType {
    OK,
    ERROR,
    REDIRECTED
};

class Response { // implements Serializable
public:
    ResponseType responseType;
    std::string message;
    std::string id;

    std::string serialize() {
        // std::cout << "Response type:" << std::to_string(responseType) << std::endl;
        // std::cout << "id:" << id << std::endl;
        // std::cout << "message:" << message << std::endl;
        std::string result = std::to_string(responseType) + " " + id + " " + message;
        return result;
    }

    Response (std::string in) {
        Response response = deserialize(in);
        responseType = response.responseType;
        message = response.message;
        id = response.id;
    }

    Response () {

    }

    static Response deserialize(std::string in) {
        Response response;
        std::istringstream stream(in);
        int responseType;
        stream >> responseType >> response.id;
        std::string messagePart = "";
        response.message = "";
        while (stream >> messagePart) response.message = response.message + messagePart + " ";
        response.responseType = ResponseType(responseType);
        return response;
    }
};

#endif