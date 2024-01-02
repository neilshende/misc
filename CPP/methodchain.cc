#include <string>
#include <memory>
#include <iostream>
//method chaining with operator() yeilds
//very sweet syntactic sugar like:
//person(Person::attr::name, "Betty")(Person::attr::occupation, "Bully")(person::attr::age, 30)
class Person {
public:
    enum class attr {
       name,
       age,
       occupation
    };
    Person& operator() (const std::string& name) {
        this->name = name;
        return *this;
    }

    Person& operator() (int age) {
        this->age = age;
        return *this;
    }

    Person& operator() (const std::string& occupation, int) {
        this->occupation = occupation;
        return *this;
    }

    Person& operator() (Person::attr attr, const std::string &value) {
        switch (attr) {
        case Person::attr::name: this->name = value; break;
        case Person::attr::age: this->age = stoi(value); break;
        case Person::attr::occupation :this->occupation = value; break;
        }
        return *this;
    }
    Person& operator() (Person::attr attr, const int &value) {
        this->age = value; //no need to check attr==age
        return *this;
    }
    void print() {
      std::cout << "Name :" << name << "\nAge :" << age << "\nOccupation :" << occupation << "\n";
    }

private:
    std::string name;
    int age;
    std::string occupation;
};

int main() {
Person person;
person(Person::attr::name, "Betty")(Person::attr::occupation, "Bully")(Person::attr::age, 30).print();
return 0;
}
