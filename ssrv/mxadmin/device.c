#include "internals.h"
#include <sys/ioctl.h>
#include <sys/poll.h>

static int gfdDev = 0;

ssize_t
WriteDev(const void *pBuffer, size_t nlen)
{
   int err;

   err = write(gfdDev, pBuffer, nlen);
   if (err == -1) {
      err = errno;
      printf("write to device failed with errno %d\n", err);
      return err;
   }

   return 0;
}

ssize_t
ReadDev(void *pBuffer, size_t nlen)
{
   int err;

   err = read(gfdDev, pBuffer, nlen);
   if (err == -1) {
      err = errno;
      printf("read from device failed with errno %d\n", err);
   }

   return err;
}

int
PollDev(void)
{
   struct pollfd ReadFD;
   int err;

   ReadFD.fd = gfdDev;
   ReadFD.events = POLLIN;
   err = poll(&ReadFD, 1, -1);
   if (err > 0) {
      return 0;
   }

   if (err == -1) {
      err = errno;
      printf("poll on device failed with errno %d\n", err);
      return err;
   }

   return EAGAIN;
}

int
CleanupDev(void)
{
   int err;

   err = ioctl(gfdDev, MXFS_IOCTL_DEINIT);
   if (err) {
      err = errno;
      printf("ioctl(MXFS_IOCTL_DEINIT) failed with errno %d\n", err);
   }

   return err;
}

void
DeinitDev(void)
{
   if (gfdDev) {
      close(gfdDev);
      gfdDev = 0;
   }

   return;
}

int
InitDev(void)
{
   int version;
   int err;

   gfdDev = open("/dev/mxfs", (O_NOCTTY | O_RDWR), 0);
   if (gfdDev == -1) {
      gfdDev = 0;
      err = errno;
      printf("open(/dev/mxfs failed with errno %d\n", err);
      return err;
   }

   err = CleanupDev();
   if (err) {
      DeinitDev();
      return err;
   }

   version = MXFS_VERSION;
   err = ioctl(gfdDev, MXFS_IOCTL_INIT, &version);
   if (err) {
      err = errno;
      printf("ioctl(MXFS_IOCTL_INIT) failed with errno %d\n", err);
      DeinitDev();
      return err;
   }

   return 0;
}

