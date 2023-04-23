#pragma once

#include <stddef.h>
#include <string>
#include <ostream>

template <typename T>
struct linked_list
{
    linked_list *prev, *next;
    T val;

    linked_list(T val, linked_list<T> *prev = nullptr, linked_list<T> *next = nullptr):
        val(val), prev(prev), next(next) {}

    linked_list *push_back(T new_val) {
        next = new linked_list(new_val, this, nullptr);
        return next;
    }
};

template <typename T>
struct linked_list_root
{
    linked_list<T> *first, *last;
    size_t size;

    linked_list_root(): first(nullptr), last(nullptr), size(0) {}

    linked_list<T> *push_back(T new_val)  {
        if(size == 0) {
            size = 1;
            first = new linked_list<T>(new_val);
            last = first;
            return first;
        }

        size++;
        last = last->push_back(new_val);
        return last;
    }

    void remove_val(T val) {
        if(size == 0) return;
        for(auto buff = first; ; buff = buff->next) {
            if(buff->val != val) {
                if(buff == last) break;
                continue;
            }
            auto prev = buff->prev,
                next = buff->next;
            if(buff != first) prev->next = next;
            else first = next;
            if(buff != last) next->prev = prev;
            else last = prev;
            size--;
            delete buff;
            return;
        }
    }

    std::string to_string() {
        std::stringstream ss;
        auto buff = first;
        for (size_t i = 0; i < size - 1; i++)
        {
            ss << buff->val << ", ";
            buff = buff->next;
        }
        ss << buff->val;
        return ss.str();
    }
};

template <typename T>
std::ostream &operator<<(std::ostream& stream, linked_list_root<T> list)
{ return stream << list.to_string(); }
