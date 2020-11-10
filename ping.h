#pragma once

#ifndef PING_HPP
#define PING_HPP
#include <unordered_set>

struct PingMessage {
    int tick;
    bool isWaiting;
    PingMessage() {
        tick = 0;
        isWaiting = false;
    }
};

#endif