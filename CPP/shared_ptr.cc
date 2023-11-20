#include <memory>
#include <iostream>
#include <chrono>
#include <ctime>
#include <random>


void CopyPtr(std::shared_ptr<int> myInt)
{
    // demonstrates that use_count increases with each copy
    std::cout << "In CopyPtr: ref count = " << myInt.use_count() << std::endl;
    std::shared_ptr<int> myCopyInt(myInt);
    std::cout << "In CopyPtr: ref count = " << myCopyInt.use_count() << std::endl;
}

void ReferencePtr(std::shared_ptr<int>& myInt)
{
    // reference count stays the same until a copy is made
    std::cout << "In ReferencePtr: ref count = " << myInt.use_count() << std::endl;
    std::shared_ptr<int> myCopyInt(myInt);
    std::cout << "In ReferencePtr: ref count = " << myCopyInt.use_count() << std::endl;
}

void MovePtr(std::shared_ptr<int>&& myInt)
{
    // demonstrates that use_count remains constant with each move
    std::cout << "In MovePtr: ref count = " << myInt.use_count() << std::endl;
    std::shared_ptr<int> myMovedInt(std::move(myInt));
    std::cout << "In MovePtr: ref count = " << myMovedInt.use_count() << std::endl;
}

int main()
{
    // demonstrates how use counts are effected between copy and move
    std::shared_ptr<int> myInt = std::make_shared<int>(5);
    std::cout << "In main: ref count = " << myInt.use_count() << std::endl;
    CopyPtr(myInt);
    std::cout << "In main: ref count = " << myInt.use_count() << std::endl;
    ReferencePtr(myInt);
    std::cout << "In main: ref count = " << myInt.use_count() << std::endl;
    MovePtr(std::move(myInt));
    std::cout << "In main: ref count = " << myInt.use_count() << std::endl;

    // since myInt was moved to MovePtr and fell out of scope on return (was destroyed),
    // we have to reinitialize myInt
    myInt.reset();
    myInt = std::make_shared<int>(5);


    return 0;
}
