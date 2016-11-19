#include <iostream>
#include <string>
#include <cstdio>
#include <math.h>
#include "dynamic_array.h"

class Foo {
public:
    Foo() : Foo(-1, "") {}
    Foo(int n, const char *s) : m_number(n), m_string(s) {}
    Foo(const Foo &other) : m_number(other.m_number), m_string(other.m_string) {}
    Foo(Foo &&other) : Foo() { swap(*this, other); }
    Foo& operator=(Foo other) { swap(*this, other); return *this; }
    friend void swap(Foo &first, Foo &second)
    {
        using std::swap;
        swap(first.m_number, second.m_number);
        swap(first.m_string, second.m_string);
    }
    void print()
    {
        std::cout << "[" << m_string << ":" << m_number << "]";
    }
public:
    int m_number;
    std::string m_string;
};

using length_t = dynamic_array<Foo>::length_t;

void print_array(dynamic_array<Foo> &foos)
{
    std::cout << "Array: [ ";
    for (length_t i = 0; i < foos.size(); i++) {
        foos[i].print();
        std::cout << " ";
    }
    std::cout << "]" << std::endl;
}

int main(int argc, char *argv[])
{
    length_t initial_capacity = 0;
    length_t num_elements = 20;
    dynamic_array<Foo> foos(initial_capacity);
    for (length_t i = 0; i < num_elements; i++) {
        char buf[64];
        sprintf(buf, "%c", 'A' + i);
        foos.push_back(Foo(i, buf));
    }
    std::cout << "Added " << foos.size() << " value(s); final capacity: " << foos.capacity() << std::endl;
    std::cout << "First item: "; foos.first().print(); std::cout << std::endl;
    std::cout << "Last item: "; foos.last().print(); std::cout << std::endl;
    std::cout << "Last valid index: " << foos.last_index() << std::endl;
    print_array(foos);
    dynamic_array<Foo> foos_temp = foos;
    length_t index = 12;
    foos_temp.shift_remove(index);
    std::cout << "Shift-remove element " << index << " from array" << std::endl;
    print_array(foos_temp);
    index = foos_temp.last_index();
    foos_temp.shift_remove(index);
    std::cout << "Shift-remove element " << index << " from array" << std::endl;
    print_array(foos_temp);

    foos_temp = foos;
    index = 9;
    foos_temp.swap_remove(index);
    std::cout << "Swap-remove element " << index << " from array" << std::endl;
    print_array(foos_temp);
    index = foos_temp.last_index();
    foos_temp.swap_remove(index);
    std::cout << "Swap-remove element " << index << " from array" << std::endl;
    print_array(foos_temp);
    return 0;
}
