#pragma once
#include <mutex>

class GSinglecton {
public:
    template<typename T>
    static T* get() {
        static std::once_flag _of;
        static T* t = nullptr;
        std::call_once(_of, []() {
            t = new T();
        });
        return t;
    }
};