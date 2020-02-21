// Suppress warning from pulling in default settings.
#pragma GCC diagnostic ignored "-Wunused-variable"

#include "internals.h"
#include <signal.h>
#include <sched.h>

static char *gpBufferDev = NULL;
static volatile int gbShutdown = 0;

static void
signal_handler(int SignalNum)
{
   if (gbShutdown == 1) {
      return;
   }

   gbShutdown = 1;
   CleanupDev();
   return;
}

static void *
WorkerThread(void *pArg)
{
   ssize_t nRead;
   size_t nBytes;
   size_t nLen;
   char *pOutBuffer;
   unsigned *pMsgLen;
   unsigned *pDataLen;
   struct mxfs_header *pMsg;
   char *pData;
   int err;

   while (1) {
      if (gbShutdown == 1) {
         break;
      }

      err = PollDev();
      if (err) {
         continue;
      }

      nRead = ReadDev(gpBufferDev, MaxBufferSize);
      if (nRead == 0) {
         continue;
      }

      if (nRead < 0) {
         break;
      }

      nLen = 0;
      nBytes = (size_t) nRead;
      while (nLen < nBytes) {
         if ((nBytes - nLen) < (2 * sizeof (unsigned))) {
            printf("error: maligned message, missing message & data length\n");
            break;
         }

         pOutBuffer = (char *) (gpBufferDev + nLen);
         pMsgLen = (unsigned *) (gpBufferDev + nLen);
         nLen += sizeof (unsigned);

         pDataLen = (unsigned *) (gpBufferDev + nLen);
         nLen += sizeof (unsigned);

         if ((nBytes - nLen) <
                        (ROUNDUP_8BYTE(*pMsgLen) + /*ROUNDUP_8BYTE*/(*pDataLen))) {
            printf("error: maligned message, missing message & data\n");
            break;
         }

         pMsg = (struct mxfs_header *) (gpBufferDev + nLen);
         nLen += sizeof (struct mxfs_header);
         nLen = ROUNDUP_8BYTE(nLen);
         pData = (char *) (gpBufferDev + nLen);
         nLen += (*pDataLen);
         /*nLen = ROUNDUP_8BYTE(nLen);*/
         if (ProcessRequest(pOutBuffer, pMsg, pData, *pDataLen)) {
            break;
         }
         if (nLen != nBytes) printf("error: garbage at end.\n");
         break;
      } /* while (nLen < nBytes) */

      sched_yield();
   } /* while (1) */

   pthread_exit(NULL);
   return NULL;
}

void
Deinit(void)
{
   DeinitDev();
   if (gpBufferDev) {
      free(gpBufferDev);
      gpBufferDev = NULL;
   }

   return;
}

int
Init(void)
{
   int err;

   gbShutdown = 0;
   gpBufferDev = malloc(MaxBufferSize);
   if (!gpBufferDev) {
      printf("failed to allocate %d memory MaxBufferSize", MaxBufferSize);
      return ENOMEM;
   }

   err = InitDev();
   if (err) {
      Deinit();
      return err;
   }

   return err;
}

int
main(void)
{
   struct sigaction sigact;
   pthread_t tid;
   int  err;

   memset(&sigact, 0, sizeof (sigact));
   sigact.sa_handler = &signal_handler;
   err = sigaction(SIGTERM, &sigact, NULL);
   if (err) {
      err = errno;
      printf("failed to add signal handler for SIGTERM, error %d", err);
      return err;
   }

   err = Init();
   if (err) {
      return err;
   }

   err = pthread_create(&tid, NULL, WorkerThread, NULL);
   if (err) {
      err = errno;
      printf("failed to create WorkerThread, error %d", err);
      Deinit();
      return err;
   }

   pthread_join(tid, NULL);
   Deinit();
   return 0;
}

