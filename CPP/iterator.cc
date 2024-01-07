#include <iostream>
#include <memory>
#include <initializer_list>  // std::initializer_list
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
        iterator& operator++(int) {iterator t(*this); ++current; return t; }
        bool operator==(const iterator& other) const { return current == other.current; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
        bool operator<(const iterator& other) const { return current < other.current; }

    private:
        T * current;
    };

    // Begin and end iterators for the class
    iterator begin() { return iterator(data); }
    iterator end() { return iterator(data+size); }
    // Copy constructor
    MyClass(MyClass& other) {
       size = other.size;
       data = new T[size];
       // copy out all the elements using the iterators.
       iterator it = begin();
       for(T value : other) {
          *it = value;
          it++;
       }
    }
    // initializer_list constructor
    MyClass(std::initializer_list<T> list) {
       size = list.size();
       data = new T[size];
       int i=0;
       for (T item: list) {
          data[i++] = item;
        }
    }

    // std::move friendly copy construnctor
    MyClass(MyClass&& other) noexcept : data(other.data), size(other.size) {// Move for rvalues
        other.data = nullptr;  // Invalidate source object to prevent double-free
        other.size = 0;
    }

    // Assignment operator
    MyClass& operator=(const MyClass& other) {
        if (this != &other) {  // Check for self-assignment
            if (size < other.size) { // reuse data otherwise.
               delete [] data;
               data = new T[other.size];
            }
            size = other.size;
            iterator it = begin();
            for(T value : const_cast<MyClass &>(other)) {
               *it++ = value; // *it = value; ++it;
            }
        }
        return *this;
    }
    // Overloaded operator<< for MyClass
    friend std::ostream& operator<<(std::ostream& os, const MyClass& obj) {
        for (T value : const_cast<MyClass &>(obj)) {
           os << value << " ";
        }
        os << "\n";
        return os;
    }
};

int main() {
    MyClass<int> myClass(100);

    // Use iterators directly for more control
    int i = 0;
    for (MyClass<int>::iterator it = myClass.begin(); it < myClass.end(); it++) {
        *it = i++;  // Modify the values through the iterator
    }

    for (int value : myClass) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    MyClass<int> yourClass(myClass);
    for (int yvalue : yourClass) {
       std::cout << yvalue << " ";
    }
    std::cout << std::endl;


    MyClass<int> copyClass(50);
    copyClass = myClass;
    for (int cvalue : copyClass) {
       std::cout << cvalue << " ";
    }
    std::cout << std::endl;

    MyClass<int> moveClass = std::move(copyClass);
    std::cout << "empty object: " << copyClass;

    std::cout << "moved object: " << moveClass;

    std::shared_ptr<MyClass<int> > obj1 = std::make_shared<MyClass<int> >(50);
    std::shared_ptr<MyClass<int> > obj2 = obj1;  // Create another shared_ptr to the same object

    for (MyClass<int>::iterator it = obj1->begin(); it < obj1->end(); it++) {
        *it = i++;
    }
    std::cout << "        obj1: " << *obj1;
    std::cout << "same as obj1: " << *obj2;

    MyClass<int> m{1,2,3,4};
    std::cout << "m initialisted with list :" << m;
    return 0;
}
