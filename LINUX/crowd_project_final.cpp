// Author - Vivek_Shende@hotmail.com
// Implements -  crowdstrike Sensor SDE coding project.

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
const int DUMP_SLEEP_SEC = 2;

// Data Structures needed:

// copy of command line arguments, needed by the pthreads.
vector<string> args_words;

// The state machine is implemented using following three flags:
volatile bool shutdown = false;
volatile bool scanning = false;
volatile bool scanning_complete = false;

class singleton {
private:
   static singleton *instance;
   static deque<string> container;
   static mutex mtx;
protected:
   singleton() {} // don't allow new

   ~singleton() { // don't allow delete
      container.clear();
   }
public:
   singleton(singleton &) = delete; //don't allow copy

   void operator=(const singleton &) = delete; //don't allow assignment

   static singleton *get_instance() {
      mtx.lock();
      if (instance == nullptr) {
         instance = new singleton;
      }
      mtx.unlock();
      return instance;
   }

   // return current accumulated size of the container.
   int container_size() {
      mtx.lock();
      int s = container.size();
      mtx.unlock();
      return s;
   }

   //  dump all accumulated matching path names.
   void dump() {
      mtx.lock();
      int count = container.size();
      for (int i=0; i<count; i++) {
         cout << container.front() << endl;
         container.pop_front();
      }
      mtx.unlock();
   }

   // push_back matching path
   void push_back(string match) {
      mtx.lock();
      container.push_back(match);
      mtx.unlock();
   }

   // clear the container
   void container_clear() {
      mtx.lock();
      container.clear();
      mtx.unlock();
   }
};
singleton* singleton::instance = nullptr;
mutex singleton::mtx;
deque<string> singleton::container;

// Thread ID of the orchestrating thread,
pthread_t finder_tid;

// The data needed by each worker thread.
typedef struct thread_data {
   string *dir;
   string *pattern;
} thread_data;

// error reporter
//TODO add bells and whistles.
void error(const string &msg) {
   cout << msg << endl;
}


void *file_finder_helper(void *args);
void *container_dumper(void *args);

class conductor {
private:
   // vector of all the thread_data
   vector<thread_data *> thread_args;
   vector<pthread_t> tids;
   pthread_t dumper_tid;
public:
   conductor() {};

   void start() {
      int count = (args_words).size();
      int res;
      for (int i=2; i<count; i++) {
         pthread_t tid;
         thread_data *x = new thread_data;
         x->dir = &(args_words[1]);
         x->pattern = &(args_words[i]);
         thread_args.push_back(x);
         res = pthread_create(&tid, NULL, file_finder_helper, x); 
         if (res == 0) {
            tids.push_back(tid);
         } else {
            error("Failed to pthread_create.");
            //TODO is this fatal? for now just go on.
         }
      }
      res = pthread_create(&dumper_tid, NULL, container_dumper, NULL);
      if (res != 0) {
         error("Unable to pthread_create.");
         //TODO is this fatal? for now just go on.
      }
  };

   void join() {
      for (auto itr=tids.begin(); itr<tids.end(); itr++) {
         int res = pthread_join(*itr, NULL);
         if (res != 0) {
            error("Unable to pthread_join.");
            //TODO is this fatal? for now just go on.
         }
      }
   };

   void join_dumper() {
      int res = pthread_join(dumper_tid, NULL);
      if (res != 0) {
         error("Unable to pthread_join.");
         //TODO handle.
      }
   };

   ~conductor() {
      for (int i=0; i<thread_args.size(); i++) {
         delete thread_args[i]; //free each thread_data
      }
      thread_args.clear(); //clear the vector of thread_data
  };

};
// Functions:

// The thread that periodically dumps available data.
// It also keeps tab on two flags:
// - shutdown - this is our hint to quit.
// - scanning_complete - this is another hint to quit, but we may still need
//   to dump the container to cout.
// It sleeps for 2 seconds between every iteration of dumping of available data.
void *container_dumper(void *args) {
   singleton *single = singleton::get_instance();
   while (!shutdown) {
      if (scanning_complete && single->container_size() == 0) {
         break;
      }

      single->dump();
      sleep(DUMP_SLEEP_SEC);
   }
   return NULL;
}

// scandir implemented using opendir and readdir - which is linux 101.
// For scanning the subdirs we use recursion. See the warning below.
// All the paths are saved as std::string, so we don't really worry about what's
// the max path length.
// The readdir loop breaks if shutdown flag is set by the CLI.
// We then closedir and exit.
static void scandir(const string &dir, const string &pattern) {
   singleton *single = singleton::get_instance();
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
         single->push_back(match);
      }
      if (dp->d_type == DT_UNKNOWN) {
         //TODO handle this by calling stat();
      }
      else if (dp->d_type == DT_DIR) {
         string subdir = dir + "/" + dp->d_name;
         scandir(subdir, pattern); //WARNING recursion.
         //Deep enough dir tree will cause stack overflow.
         //Will need to implement using stack data structure to handle
         //that case.
      }
   }
   closedir(dfd);
}

// We receive the thread_data as args which contain dir and pattern.
// Invoke scandir with those.
void *file_finder_helper(void *args) {
   thread_data *data = (thread_data *)args;
   scandir(*(data->dir), *(data->pattern));
   return NULL;
}

// This is the orchestrating thread that starts:
// - all the helper threads to start scanning the directories
// - and the dumper thread.
// Then it waits for all the thread to finish executing by calling
// thread_join()
// It then performs the cleanup of the three data structures.
// - All the thread_data and their vector.
// - Vector of command line words.
// - the container that collects the matching file names
static void *file_finder(void *args) {
   conductor sw;
   singleton *single = singleton::get_instance();
   sw.start();

   sw.join();
   args_words.clear(); //clear the vector of command line arguments
   scanning = false;
   scanning_complete = true;

   sw.join_dumper();
   single->container_clear(); //clean the container in case of shutdown
   return NULL;
}

// The main entry point implements the Command line Interface
// state machine and launches the file-finder functionality.
// First it converts the command line to vector of words.
// Then it sets up the needed data structures and launches
// the orchestrating thread.
// Then it goes in a loop reading user input.
// For exit command it uses the shutdown flag to inform all workers to stop. The
// orchestrating thread takes care of cleanups.
// For dump command, it reuses the helper function that dumps available data.
int main(int argc, char *argv[]) {
   if (argc < 3) {
      error("Missing arguments.");
      return 1;
   }
   singleton *single = singleton::get_instance();
   string line;
   args_words.clear();
   for (int i=0; i<argc; i++) {
      args_words.push_back(argv[i]);
   }
   scanning = true;
   int res = pthread_create(&finder_tid, NULL, file_finder, NULL);
   if (res != 0) {
      error("Fatal: Unable to pthread_create.");
      return 1;
   }
   string prompt = "scanning> ";
   while (!shutdown) {
      if (scanning_complete) {
          if (single->container_size() == 0) {
             // we can exit() now, but let's give user chance to type "exit"
             prompt = "scan and dump complete> ";
          } else {
             prompt = "scan complete> ";
          }
      }
      cout << prompt;
      getline(cin, line);
      if (!cin) {
         //handle this like "exit"
         shutdown = true;
         if (scanning || scanning_complete) {
            int res = pthread_join(finder_tid, NULL);
            if (res != 0) error("Unable to pthread_join.");
         }
         error("EOF");
         break;
      }
      string word;
      stringstream ss(line);
      vector<string> words;
      words.clear();
      while (ss >> word) {
         words.push_back(word);
      }
      if (words.size() == 0) continue;
      if (words[0] == "dump") {
         single->dump();
      } else if (words[0] == "exit") {
         shutdown = true;
         if (scanning || scanning_complete) {
            int res = pthread_join(finder_tid, NULL);
            if (res != 0) error("Unable to pthread_join.");
         }
      } else {
         error("Invalid command");
      }
   }
   assert(single->container_size()==0);
   assert(args_words.size()==0);
   return 0;
}
