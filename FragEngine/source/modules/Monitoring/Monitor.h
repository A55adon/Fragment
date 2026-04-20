#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <functional>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include "core/DataTypes.h"
#include "core/DefaultFunctions.h"

class BaseBuffer : public std::streambuf {
public:
    std::string storage;
    std::function<void(const std::string&)> onEntry;
    std::streambuf* forward = nullptr;

    BaseBuffer(std::streambuf* forwardTo = nullptr)
        : forward(forwardTo) {
    }

protected:
    int overflow(int c) override {
        if (c == EOF) return !EOF;

        char ch = static_cast<char>(c);
        storage += ch;

        if (forward)
            forward->sputc(ch);

        if (onEntry)
            onEntry(std::string(1, ch));

        return c;
    }

    std::streamsize xsputn(const char* s, std::streamsize n) override {
        storage.append(s, n);

        if (forward)
            forward->sputn(s, n);

        if (onEntry)
            onEntry(std::string(s, n));

        return n;
    }
};

template<typename T = std::string>
class Buffer {
private:
    BaseBuffer buf;
    std::ostream stream;

public:
    Buffer(std::ostream& forwardTo = std::cout)
        : buf(forwardTo.rdbuf()), stream(&buf) {
    }

    template<typename U>
    Buffer& operator<<(const U& value) {
        stream << value;
        return *this;
    }

    void setOnEntry(std::function<void(const std::string&)> fn) {
        buf.onEntry = fn;
    }

    void flush() {
        stream.flush();
    }

    void clear() {
        buf.storage.clear();
    }

    std::string str() const {
        return buf.storage;
    }

    void dumpToFile(const std::string& filename, bool append = false) {
        std::ofstream file(filename, append ? std::ios::app : std::ios::out);
        if (file)
            file << buf.storage;
    }
};

template<typename T>
class VarCapture {
public:
    std::vector<T> history;
    std::function<void(const T&)> onEntry = nullptr;

    VarCapture(std::function<T()> getter,
        std::chrono::milliseconds interval, bool store = false)
        : getter(getter), interval(interval), running(true)
    {
        worker = std::thread([this]() {
            while (running) {
                auto start = std::chrono::steady_clock::now();

                T value = this->getter();
                push(value);

                std::this_thread::sleep_until(start + this->interval);
            }
            });
    }

    ~VarCapture() {
        running = false;
        if (worker.joinable())
            worker.join();
    }

    void push(const T& value) {
        if(_store)
            history.push_back(value);
        if (onEntry)
            onEntry(value);
    }

    bool _store = false;

private:
    std::function<T()> getter;
    std::chrono::milliseconds interval;
    std::atomic<bool> running;
    std::thread worker;
};

class Monitor {
public:

	Monitor();

private:

	Buffer<std::string> _debugStream;
	Buffer<std::string> _errorStream;
	Buffer<std::string> _dependenciesStream;

};

