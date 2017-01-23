#include <List.hpp>


template <typename T>
struct list_elem {
    class list_elem *next;
    T value;
}

template <typename T>
class List {

private:
    list_elem<T> *head;
    list_elem<T> *back;

public:
    List() : head(nullptr), back(nullptr) {}
    ~List() {
        while (struct list_elem<T> *e = head; e; e = next) {
            struct list_elem<T> *next = e->next;
            delete e;
        }
    }

    List push(const T& value) {
        auto e = new struct list_elem<T>;
        e->value = value;
        e->next  = nullptr;

        if (!back) {
            head = e;
            back = e;
        } else {
            back->next = e;
        }
    }
};
