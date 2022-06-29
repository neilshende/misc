#include <pthread.h>
#include <signal.h>
#include <sys/select.h>

// the muticies, protectors of the shared resources
pthread_mutex_t coutLock;
pthread_mutex_t inQueueLock;
pthread_mutex_t outQueueLock;
// the shared data
std::list< std::string > inQueue;
std::list< std::string > outQueue;

struct thread_detail { // information to pass to worker threads
 unsigned long num;
};

extern "C" {
    void *workerThread(void *threadarg);
}

void *workerThread(void *threadarg) 
{
   struct thread_detail *my_data;
   my_data = (thread_detail *) threadarg;
   int taskid = my_data->num;
   std::stringstream ss; ss<<taskid; std::string taskString(ss.str());

   bool somethingTodo=true;
   while (somethingTodo) // keep on working until inQueue is empty 
    {
      pthread_mutex_lock( &inQueueLock );
      std::string workOnMe;
      if (inQueue.size()==0) { somethingTodo=false; }
      else
        {
         workOnMe = inQueue.front();
         inQueue.pop_front();
        }
      pthread_mutex_unlock( &inQueueLock );

      if (!somethingTodo) break;
      workOnMe = "thread " + taskString + " worked on " + workOnMe;
      // let's pretend this takes some time, add a delay to the computation
      struct timeval timeout;
      timeout.tv_sec = 0;  timeout.tv_usec = 100000; // 0.1 second delay 
      select( 0, NULL, NULL, NULL, & timeout );

      pthread_mutex_lock( &outQueueLock );
      outQueue.push_back( workOnMe );
      pthread_mutex_unlock( &outQueueLock );
    }

   pthread_exit(NULL);
}


int main (int argc, char *argv[])
{
  unsigned long comp_DONE=0; 
  unsigned long comp_START=0;
  // set-up the mutexes
  pthread_mutex_init( &coutLock, NULL );
  pthread_mutex_init( &inQueueLock, NULL );
  pthread_mutex_init( &outQueueLock, NULL );

  if (argc != 3) { std::cout<<"Program requires two arguments: (1) number of threads to use,"
                           " and (2) tasks to accomplish.\n"; exit(1); }
  unsigned long NUM_THREADS(atoi( argv[1] ));
  unsigned long comp_TODO(atoi(argv[2]));
  std::cout<<"Program will have "<<NUM_THREADS<<" threads, working on "<<comp_TODO<<" things \n";
  for (unsigned long i=0; i<comp_TODO; i++) // fill inQueue will rubbish data since this isn't an actual computation...
   {
    std::stringstream ss; 
    ss<<"task "<<i; 
    inQueue.push_back(ss.str());
   }

  // start the worker threads
  std::list< pthread_t* > threadIdList; // just the thread ids
  std::list< thread_detail > thread_table; // for keeping track of information on the various threads we'll create
  for (unsigned long i=0; i<NUM_THREADS; i++) // start the threads
   {
    pthread_t *tId( new pthread_t );   threadIdList.push_back(tId);
    thread_detail Y; Y.num=i; thread_table.push_back(Y);
    int rc( pthread_create( tId, NULL, workerThread, (void *)(&(thread_table.back() )) ) );
    if (rc) { std::cout<<"ERROR; return code from pthread_create() "<<comp_START<<"\n"; std::cout.flush();
              exit(-1); }
   }
  // now we wait for the threads to terminate, perhaps updating the screen with info as we go. 
  std::string stringOut;
  while (comp_DONE != comp_TODO)
   {
         // poll the queue to get a status update on computation
         pthread_mutex_lock(&inQueueLock);
         comp_START = comp_TODO -  inQueue.size();
         pthread_mutex_unlock(&inQueueLock);
         pthread_mutex_lock(&outQueueLock);
         comp_DONE = outQueue.size();
         pthread_mutex_unlock(&outQueueLock);

         // update for users
         pthread_mutex_lock(&coutLock);
         for (unsigned long i=0; i<stringOut.length(); i++) std::cout<<"\b";
         std::stringstream ss; ss<<"started "<<comp_START<<" completed "<<comp_DONE<<" of "<<comp_TODO;  
         stringOut = ss.str();  std::cout<<stringOut;  std::cout.flush();
         pthread_mutex_unlock(&coutLock);

         // wait one second per update
         struct timeval timeout;
         timeout.tv_sec = 1;  timeout.tv_usec = 0;  
         select( 0, NULL, NULL, NULL, & timeout );
        } // big while loop

   // call join to kill all worker threads
   std::list< pthread_t* >::iterator i(threadIdList.begin());
  while (i!=threadIdList.end())
   {
    if (pthread_join( *(*i), NULL)!=0) { std::cout<<"Thread join error!\n"; exit(1); }
    delete (*i);
    threadIdList.erase(i++);  
   }
  std::cout<<"\n";

   // let the user know what happened
   for (std::list< std::string >::iterator i=outQueue.begin(); i!=outQueue.end(); i++)
    {
     std::cout<<(*i)<<"\n";
    }
    // clean-up
    pthread_mutex_destroy(&coutLock);
    pthread_mutex_destroy(&inQueueLock);  
    pthread_mutex_destroy(&outQueueLock);  
    // pthread_exit(NULL);
}
