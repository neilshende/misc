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

// Data Structures needed:

// copy of command line arguments, needed by the pthreads.
vector<string> args_words;

// The state machine is implemented using following three flags:
volatile bool shutdown = false;
volatile bool scanning = false;
volatile bool scanning_complete = false;

// The container for collecting filenames matching the pattern.
// Shared by all threads...
deque<string> container;

// ... And synchronized using mutex mtx.
mutex mtx;

// Thread ID of the orchestrating thread,
pthread_t finder_tid;

// The data needed by each worker thread.
typedef struct thread_data {
   string *dir;
   string *pattern;
} thread_data;

// And vector of all the thread_data
vector<thread_data *> thread_args;

// Functions:

// error reporter
//TODO add bells and whistles.
void error(const string &msg) {
   cout << msg << endl;
}

// helper thread that dumps available file names collected.
// synchronization is done using the mutex.
static void container_dumper_helper() {
   mtx.lock();
   int count = container.size();
   for (int i=0; i<count; i++) {
      cout << container.front() << endl;
      container.pop_front();
   }
   mtx.unlock();
}

// The thread that periodically dumps available data.
// It also keeps tab on two flags:
// - shutdown - this is our hint to quit.
// - scanning_complete - this is another hint to quit, but we may still need
//   to dump the container to cout.
// It sleeps for 2 seconds between every iteration of dumping of available data.
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

// scandir implemented using opendir and readdir - which is linux 101.
// For scanning the subdirs we use recursion. See the warning below.
// All the paths are saved as std::string, so we don't really worry about what's
// the max path length.
// The readdir loop breaks if shutdown flag is set by the CLI.
// We then closedir and exit.
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
static void *file_finder_helper(void *args) {
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
   int count = (args_words).size();
   int res;
   vector<pthread_t> tids;
   for (int i=2; i<count; i++) {
      pthread_t tid;
      thread_data *x = new thread_data;
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
       delete thread_args[i]; //free each thread_data
   }
   thread_args.clear(); //clear the vector of thread_data
   args_words.clear(); //clear the vector of command line arguments
   scanning = false;
   scanning_complete = true;
   if (res2==0) res = pthread_join(dumper_tid, NULL);
   //TODO handle res!=0
   container.clear(); //clean the container in case of shutdown
   return NULL;
}

// The main entry point implements the Command line Interface
// state machine.
// It accepts input from the console.
// Converts the input line to vector of words.
// For file-finder command it sets up the needed data structures and launches
// the orchestrating thread.
// For exit command it uses the shutdown flag to inform all workers to stop. The
// orchestrating thread takes care of cleanups.
// For dump command, it reuses the helper function that dumps available data.
int main() {
   string line;
   while (!shutdown) {
      cout << "input command> ";
      getline(cin, line);
      if (!cin) {
         //TODO handle this like "exit"
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
      } else {
         error("Invalid command");
      }
   }
   assert(container.size()==0);
   assert(thread_args.size()==0);
   assert(args_words.size()==0);
   return 0;
}
