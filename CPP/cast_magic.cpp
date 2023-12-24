#include <iostream>
using namespace std;
class hello {
private:
   int magic;
public:
  void print() {
     cout << "Hello " << magic << endl;
   }
  hello() { magic = 911911;}
  ~hello() { magic = 199199;}
  int getmagic() { return magic;}
  bool valid() { return magic==911911;}
};
int main(int argc, char *argv[]) {
   hello *h = new hello;
   long c = (long)h;
   hello *hc = (hello *)c;
   cout << "c style cast " << hc->getmagic() << " valid=" << hc->valid() << endl;
   long p = reinterpret_cast<long>(h);
   cout << "c++ cast " << reinterpret_cast<hello *>(p)->getmagic() << " valid=" << reinterpret_cast<hello *>(p)->valid() << endl;
   hello *h2 = reinterpret_cast<hello *>(p+100);
   cout << h2->getmagic()<< " valid=" << h2->valid() <<endl;
   h2->print();
   delete h;
   cout <<  reinterpret_cast<hello *>(p)->getmagic() << " valid=" << reinterpret_cast<hello *>(p)->valid() << endl;
   return 0;
}
