#ifndef __CPP_LIST_H__
#define __CPP_LIST_H__

#include <resea.h>


template <typename T>
class ListElement {
public:
    class ListElement *next;
    T *value;
};

// TODO: add a mutex
template <typename T>
class List {

private:
    ListElement<T> *head;
    ListElement<T> *back;

public:
    List() : head(nullptr), back(nullptr) {}
    ~List() {
        for (ListElement<T> *e = head; e;) {
            ListElement<T> *next = e->next;
            delete e;
            e = next;
        }
    }

    List& push(const T& value) {
        auto e = new ListElement<T>;
        e->value = new T(value);
        e->next  = nullptr;

        if (!back) {
            head = e;
            back = e;
        } else {
            back->next = e;
            back = e;
        }

        return *this;
    }

    class iterator {
    private:
        ListElement<T> *ptr;
    public:
        iterator(ListElement<T> *ptr) : ptr(ptr) {}

        const T& operator*() {
            return *(ptr->value);
        }

        bool operator!=(const iterator& iter) {
            return ptr != iter.ptr;
        }

        iterator operator++() {
            ptr = ptr->next;
            return iterator(ptr);
        }
    };

    iterator begin() {
        return iterator(head);
    }

    iterator end() {
        return iterator(nullptr);
    }
};

#endif
