#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string.h>
#include <iostream>
using namespace std;

string parent(string path) {
  //if (path =="/") return path;
  if (path.rfind('/') == 0) return "/";
  return path.substr(0, path.rfind('/'));
}

string helper(string path, string token) {
  //std::cout << "token=" << token << std::endl;
  if (token == ".") return path;
  if (token == "..") return parent(path);
  return path+"/"+token;
}
std::string removeDuplicateSlashes(char *arr) {
       int i=0;
       int j=1;
       int n = strlen(arr);

        while (j< n) {
            if (arr[i]=='/' && arr[i]==arr[j]) j++; else arr[++i] = arr[j++];
        }

        arr[i+1] = '\0';
        return std::string(arr);
}
string cd(char * cwd, char * cmd) {
   if (cmd[0] == '/') { //cmd is absolute path. cwd is irrelevant.
        char root[] = ""; //cd will add slash as needed
        return cd(root, cmd+1);
   }
   int n= strlen(cwd);
   while (n>1 && cwd[n-1]=='/') cwd[--n]='\0'; //strip all trailing slashes.
   char *y = std::strtok(cmd, "/");
   string ret= string(cwd);
   while (y) {
      ret = helper(ret, y);
      y = strtok(NULL, "/");
   }
   return ret;
}

int main() {
char foo[] = "/this///is/a/test//of/fix";
char bar[] = "/this/is/another/test//////";
std::cout << foo << std::endl;
std::cout << removeDuplicateSlashes(foo) <<std::endl;
std::cout << bar << std::endl;
std::cout << removeDuplicateSlashes(bar) << std::endl;
char s[] ="/this/is/a/test/foo//////////////";
char c[] = "../../..//bar";
std::cout << "parent of /foo is " << parent(string("/foo")) << endl;
std::cout << cd(s, c) <<std::endl;
char ss[] = "/this/is/a/test";
char cc[] = "/////////../../.././that/is/././right/..";
std::cout << cd(ss, cc) << std::endl;
return 0;
}
