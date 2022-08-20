#include <stdlib.h>
#include <stdio.h>
#include <cstdio>
#include <vector>
#include <deque>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <pthread.h>
#include <mutex>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

using namespace std;
vector<string> args_words;
volatile bool shutdown = false;
volatile bool scanning = false;
volatile bool scanning_complete = false;
deque<string> container;
mutex mtx;
pthread_t finder_tid;
typedef struct thread_data {
   string *dir;
   string *pattern;
} thread_data;
vector<thread_data *> thread_args;

void error(const string &msg) {
   cout << msg << endl;
}

static void container_dumper_helper() {
   mtx.lock();
   int count = container.size();
   for (int i=0; i<count; i++) {
      cout << container.front() << endl;
      container.pop_front();
   }
   mtx.unlock();
}

static void *container_dumper(void *args) {
   while (!shutdown) {
      mtx.lock();
      if (scanning_complete && container.size() == 0) {
         mtx.unlock();
         break;
      }
      mtx.unlock();

      container_dumper_helper();
      sleep(2); //TODO remove hard-coding
   }
   return NULL;
}

static void scandir(const string &dir, const string &pattern) {
   struct dirent *dp;
   DIR *dfd = opendir(dir.c_str());
   if (dfd == NULL) {
      error("Unable to opendir.");
      return;
   }
   while((dp = readdir(dfd)) != NULL) {
      if (shutdown) break; //Time to cleanup and quit.
      if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
         continue;
      }
      if (strstr(dp->d_name, pattern.c_str()) != NULL) {
         string match = dir + "/" + string(dp->d_name);
         mtx.lock();
         container.push_back(match);
         mtx.unlock();
      }
      if (dp->d_type == DT_UNKNOWN) {
         //TODO handle this by calling stat();
      }
      else if (dp->d_type == DT_DIR) {
         string subdir = dir + "/" + dp->d_name;
         scandir(subdir, pattern); //recursion.
      }
   }
   closedir(dfd);
}

static void *file_finder_helper(void *args) {
   thread_data *data = (thread_data *)args;
   scandir(*(data->dir), *(data->pattern));
   return NULL;
}

static void *file_finder(void *args) {
   int count = (args_words).size();
   int res;
   vector<pthread_t> tids;
   for (int i=2; i<count; i++) {
      pthread_t tid;
      thread_data *x = (thread_data *)malloc(sizeof(thread_data));
      x->dir = &(args_words[1]);
      x->pattern = &(args_words[i]);
      thread_args.push_back(x);
      res = pthread_create(&tid, NULL, file_finder_helper, x);
      if (res==0) tids.push_back(tid);
   }
   pthread_t dumper_tid;
   scanning_complete = false;
   int res2 = pthread_create(&dumper_tid, NULL, container_dumper, NULL);
   //TODO  handle res2!=0
   for (auto itr=tids.begin(); itr<tids.end(); itr++) {
      res = pthread_join(*itr, NULL);
      //TODO handle res!=0
   }
   for (int i=0; i<thread_args.size(); i++) {
       free(thread_args[i]); //free each thread_data
   }
   thread_args.clear();
   args_words.clear();
   scanning = false;
   scanning_complete = true;
   if (res2==0) res = pthread_join(dumper_tid, NULL);
   //TODO handle res!=0
   container.clear();
   return NULL;
}

int main() {
   string line;
   while (!shutdown) {
      cout << "input command> ";
      getline(cin, line);
      string word;
      stringstream ss(line);
      vector<string> words;
      words.clear();
      while (ss >> word) {
         words.push_back(word);
      }
      if (words.size() == 0) continue;
      if (words[0] == "file-finder") {
         if (!scanning) {
            scanning = true;
            args_words.clear();
            for (int i=0; i<words.size(); i++) {
               args_words.push_back(words[i]);
            }
            int res = pthread_create(&finder_tid, NULL, file_finder, NULL);
            //TODO handle res!=0
         } else {
            error("Already scanning");
         }
      } else if (words[0] == "dump") {
         if (!scanning) {
            error("Not scanning");
         } else {
            container_dumper_helper();
         }
      } else if (words[0] == "exit") {
         shutdown = true;
         if (scanning) {
            int res = pthread_join(finder_tid, NULL);
            //TODO handle res!=0
         }
         container.clear();
      } else {
         error("Invalid command");
      }
   }
   return 0;
}
