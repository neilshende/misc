#include <iostream>       // std::cout
#include <stack>          // std::stack
#include <vector>         // std::vector
#include <deque>          // std::deque
void toh(int n, std::stack<int> &from, std::stack<int> &to, std::stack<int> &temp)
{
   if (n==1) {
       to.push(from.top());
       from.pop();
       return;
   }
   toh(n-1, from, temp, to);
   to.push(from.top());
   from.pop();
   toh(n-1, temp, to, from);
}
int main ()
{

  std::stack<int> first;                    // empty stack
  std::stack<int> second;
  std::stack<int> third;

  for (int i = 1; i < 11; i++) first.push(i);
  toh(10, first, second, third);
  std::cout << "size of first: " << first.size() << '\n';
  std::cout << "size of second: " << second.size() << '\n';
  std::cout << "size of third: " << third.size() << '\n';
  std::cout << "Popping out elements...";
  while (!second.empty())
  {
     std::cout << ' ' << second.top();
     second.pop();
  }
  std::cout << '\n';
  return 0;
}

