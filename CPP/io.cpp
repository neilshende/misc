#include <iostream>
#include <fstream>
#include <string>
using namespace std;

void open_and_write(string &path, string &buffer)
{
   ofstream of;
   of.open(path);
   of << buffer;
   of.close();
}

void open_and_read(string &path)
{
   string line;
   ifstream fi(path);
   if (fi.is_open()) {
      while (getline(fi, line)) {
         cout << line << endl;
      }
   }
   fi.close();
}

int main(int argc, char *argv[])
{
   string path("/tmp/foobar");
   string buffer("This is a test\nA quick brwon fox jumped over a lazy dog\n");
   open_and_write(path, buffer);
   open_and_read(path);
   return 0;
}
