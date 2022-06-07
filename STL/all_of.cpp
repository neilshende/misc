// all_of
// any_of none_of for_each
// work similarly.
#include <iostream>     // std::cout
#include <algorithm>    // std::all_of
#include <array>        // std::array
bool is_odd(int i) {
   return i%2;
}
struct myclass {           // function object type:
  void operator() (int i) {std::cout << ' ' << i;}
} functor;

int main () {
  std::array<int,8> foo = {3,5,7,11,13,17,19,23};

  //if ( std::all_of(foo.begin(), foo.end(), [](int i){return i%2;}) )
  if ( std::all_of(foo.begin(), foo.end(), is_odd) )
    std::cout << "All the elements are odd numbers.\n";

  std::for_each(foo.begin(), foo.end(), functor);

  std::cout << "\nOld way\n";
  for (auto it=foo.begin(); it != foo.end(); it++) std::cout << " " << *it;
  std::cout << "\n";

  return 0;
}
