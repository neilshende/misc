#include <iostream>
//#include <iterator>

// Sample class to iterate over
template <typename T>
class MyClass {
public:
    T *data;
    int size;

    MyClass(const int max): size(max) { data = new T[size]; }
    ~MyClass() { delete [] data;}

    // Define the iterator type as a nested class
    //class iterator : public std::iterator<std::forward_iterator_tag, T> {
    class iterator {
    public:
        iterator(T *it) : current(it) {}

        int& operator*() const { return *current; }
        iterator& operator++() { ++current; return *this; }
        bool operator==(const iterator& other) const { return current == other.current; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
        bool operator<(const iterator& other) const { return current < other.current; }

    private:
        T * current;
    };

    // Begin and end iterators for the class
    iterator begin() { return iterator(&data[0]); }
    iterator end() { return iterator(&data[size]); }
};

int main() {
    MyClass<int> myClass(100);

    // Use iterators directly for more control
    int i = 0;
    for (MyClass<int>::iterator it = myClass.begin(); it < myClass.end(); ++it) {
        *it = i++;  // Modify the values through the iterator
    }

    for (int value : myClass) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}
