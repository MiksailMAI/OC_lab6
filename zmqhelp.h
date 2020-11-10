#pragma once

#ifndef ZMQHELP_HPP
#define ZMQHELP_HPP

#include <zmq.hpp>
#include <string>
#include <cstring>
#include <iostream>

bool DEBUG = false;


class ZmqSocketClient {
private:
    zmq::context_t* context;
    zmq::socket_t* client;
    int port;

public:
    ZmqSocketClient(int portIn) {
        zmq::context_t context;
        this->port = portIn;
        this->context = new zmq::context_t(1);
        this->client = new zmq::socket_t(*this->context, ZMQ_PUSH);
        this->client->connect(get_port_name().c_str());
        if (DEBUG) std::cout << "ZmqSocketClient: connected to socket on port: " << portIn << "\n";
    }

    bool send_message(const std::string& message) {
        zmq::message_t msg(message.c_str(), message.size());
        bool res = this->client->send(msg);
        if(!res) {
            return false;
        }
        if (DEBUG) std::cout << "ZmqSocketClient: sended message to port " << this->port << ": " << message << std::endl; 
        return true;
    }

    ~ZmqSocketClient() {
        client->close();
        context->close();
    }

private:
    std::string get_port_name() {
        return "tcp://localhost:" + std::to_string(this->port);
    }
};

class ZmqSocketServer {
private:
    zmq::context_t* context;
    zmq::socket_t* server;
    int port;

public:

    ZmqSocketServer(int portIn) {
        this->port = portIn;
        this->context = new zmq::context_t(1);
        this->server = new zmq::socket_t(*this->context, ZMQ_PULL);
        this->server->bind(get_port_name().c_str());
        if (DEBUG) std::cout << "ZmqSocketServer: binded socket on port: " << portIn << "\n";
    }

    void* getContext() {
        return this->context;
    }
    void* getSocket() {
        return this->server;
    }

    ~ZmqSocketServer() {
        server->close();
        context->close();
    }

std::string recieve_message() {
    zmq::message_t msg;
    this->server->recv(&msg);
    std::string result(static_cast<char*>(msg.data()), msg.size());
    if (DEBUG) std::cout << "ZmqSocketServer: received message on port " << this->port << ": " << result << std::endl;
    return result;
}

private:
std::string get_port_name() {
    return "tcp://*:" + std::to_string(this->port);
}
};

#endif