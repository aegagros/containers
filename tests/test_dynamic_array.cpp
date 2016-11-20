#include <iostream>
#include <string>
#include <cstdio>
#include <math.h>
#include "dynamic_array.h"

class Foo {
public:
    static int DefAllocs;
    static int Allocs;
    static int Copies;
    static int Swaps;

    Foo() : Foo(-1, "") { Allocs--; DefAllocs++; }
    Foo(int n, const char *s) : m_number(n), m_string(s) { Allocs++; }
    Foo(const Foo &other) : m_number(other.m_number), m_string(other.m_string) { Copies++; }
    Foo(Foo &&other) : Foo() { swap(*this, other); }
    Foo& operator=(Foo other) { swap(*this, other); return *this; }
    friend void swap(Foo &first, Foo &second)
    {
        using std::swap;
        swap(first.m_number, second.m_number);
        swap(first.m_string, second.m_string);
        Swaps++;
    }
    void print()
    {
        std::cout << "[" << m_string << ":" << m_number << "]";
    }
    static void Reset_stats()
    {
        DefAllocs = 0;
        Allocs = 0;
        Copies = 0;
        Swaps = 0;
    }
    static void Print_stats(const char *msg)
    {
        printf("%s: default allocations:%d allocations:%d copies:%d swaps:%d\n",
               msg,
               DefAllocs,
               Allocs,
               Copies,
               Swaps);
    }

public:
    int m_number;
    std::string m_string;
};


int Foo::DefAllocs = 0;
int Foo::Allocs = 0;
int Foo::Copies = 0;
int Foo::Swaps = 0;
using length_t = dynamic_array<Foo>::length_t;

void print_array(dynamic_array<Foo> &foos)
{
    std::cout << std::endl << "Array: [ ";
    for (length_t i = 0; i < foos.size(); i++) {
        foos[i].print();
        std::cout << " ";
    }
    std::cout << "]" << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
    char buf[64];
    length_t initial_capacity = 1000;
    length_t num_elements = 1000;
    Foo::Reset_stats();
    dynamic_array<Foo> foos(initial_capacity);
    for (length_t i = 0; i < num_elements; i++) {
        sprintf(buf, "%c", 'A' + i % 26);
        foos.emplace_back(i, buf);
    }
    Foo::Print_stats("Initialized an array");
    std::cout << "Added " << foos.size() << " value(s); final capacity: " << foos.capacity() << std::endl;
    std::cout << "First item: "; foos.first().print(); std::cout << std::endl;
    std::cout << "Last item: "; foos.last().print(); std::cout << std::endl;
    std::cout << "Last valid index: " << foos.last_index() << std::endl;
    print_array(foos);

    Foo::Reset_stats();
    dynamic_array<Foo> foos_temp = foos;
    sprintf(buf, "Copied an array of %d item(s)", foos.size());
    Foo::Print_stats(buf);

    length_t index = 12;
    foos_temp.shift_remove(index);
    std::cout << "Shift-remove element " << index << " from array" << std::endl;
    print_array(foos_temp);
    index = foos_temp.last_index();
    foos_temp.shift_remove(index);
    std::cout << "Shift-remove element " << index << " from array" << std::endl;
    print_array(foos_temp);

    Foo::Reset_stats();
    foos_temp = std::move(foos);
    sprintf(buf, "Move-assigned an array of %d item(s) - original array size: %d", foos_temp.size(), foos.size());
    Foo::Print_stats(buf);

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
