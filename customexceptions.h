#pragma once

#ifndef CUSTOM_EXCEPTIONS_HPP
#define CUSTOM_EXCEPTIONS_HPP

#include <exception>
#include <stdio.h>

class AlreadyExistsException : public std::exception {
    const char* what() const throw() {
        return "Error: Already exists";
    }
};

class ParentNotFoundException : public std::exception {
    const char* what() const throw() {
        return "Error: Parent not found";
    }
};

class ParentUnavailableException : public std::exception {
    const char* what() const throw() {
        return "Error: Parent is unavailable";
    }
};

class NotExistsException : public std::exception {
private:
    int id;
public:
    NotExistsException(int idIn) {
        this->id = idIn;
    }

    const char* what() const throw() {
        char* s = new char[100];
        sprintf(s, "Error:%d: Not found", id);
        return s;
    }
};

class NotAvailableException : public std::exception {
private:
    int id;
public:
    NotAvailableException(int idIn) {
        this->id = idIn;
    }
    const char* what() const throw() {
        char* s;
        sprintf(s, "Error:%d: Node is unavailable", id);
        return s;
    }
};

class ForkException : public std::exception {
    const char* what() const throw() {
        return "Error: Can not fork";
    }
};

#endif