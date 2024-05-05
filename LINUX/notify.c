#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define EVENT_SIZE  ( sizeof(struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )

void handle_event(struct inotify_event *event) {
  // Print event details
  printf("Event: ");
  if (event->wd > 0)
    printf("wd=%d; ", event->wd);

  if (event->mask & IN_OPEN)
    printf("IN_OPEN ");
  if (event->mask & IN_CLOSE)
    printf("IN_CLOSE ");
  if (event->mask & IN_MOVED_FROM)
    printf("IN_MOVED_FROM ");
  if (event->mask & IN_MOVED_TO)
    printf("IN_MOVED_TO ");
  if (event->mask & IN_CREATE)
    printf("IN_CREATE ");
  if (event->mask & IN_DELETE)
    printf("IN_DELETE ");
  if (event->mask & IN_MODIFY)
    printf("IN_MODIFY ");
  if (event->mask & IN_DELETE_SELF)
    printf("IN_DELETE_SELF ");
  if (event->mask & IN_UNMOUNT)
    printf("IN_UNMOUNT ");
  if (event->mask & IN_Q_OVERFLOW)
    printf("IN_Q_OVERFLOW ");
  if (event->mask & IN_IGNORED)
    printf("IN_IGNORED ");
  if (event->mask & IN_ACCESS)
    printf("IN_ACCESS ");
  if (event->mask & IN_ATTRIB)
    printf("IN_ATTRIB ");
  if (event->mask & IN_DONT_FOLLOW)
    printf("IN_DONT_FOLLOW ");
  if (event->mask & IN_MASK_ADD)
    printf("IN_MASK_ADD ");
  if (event->mask & IN_ISDIR)
    printf("IN_ISDIR ");
  if (event->mask & IN_ONESHOT)
    printf("IN_ONESHOT ");
  if (event->mask & IN_EXCL_UNLINK)
    printf("IN_EXCL_UNLINK ");
  if (event->mask & IN_ONLYDIR)
    printf("IN_ONLYDIR ");
  if (event->mask & IN_DONT_CREATE)
    printf("IN_DONT_CREATE ");
  if (event->mask & IN_WATCH_QUEUE_FULL)
    printf("IN_WATCH_QUEUE_FULL ");
  if (event->mask & IN_COOKIE_WATCH)
    printf("IN_COOKIE_WATCH ");

  // Print specific details based on event type
  if ( event->mask & IN_MOVED_FROM || event->mask & IN_MOVED_TO) {
    printf(" (%s)", event->name);
  } else if (event->mask & IN_CREATE || event->mask & IN_DELETE) {
    if ( event->name )
      printf(" (%s)", event->name);
  }

  printf("\n");
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <directory>\n", argv[0]);
    exit(1);
  }

  // Initialize inotify instance
  int inotify_fd = inotify_init();
  if (inotify_fd == -1) {
    perror("inotify_init");
    exit(1);
  }

  // Watch the directory for events
  int wd = inotify_add_watch(inotify_fd, argv[1], IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_MODIFY );
  if (wd == -1) {
    perror("inotify_add_watch");
    exit(1);
  }

  // Allocate memory for reading events
  char buffer[EVENT_BUF_LEN];

  // Continuously monitor for events
  while (1) {
    ssize_t bytes_read = read(inotify_fd, buffer, EVENT_BUF_LEN);
    if (bytes_read == -1) {
      perror("read");
      exit(1);
    } else if (bytes_read == 0) {
      // Handle no events read (optional)
      printf("No events for now...\n");
    } else {
      // Process events
      int i = 0;
      while (i < bytes_read) {
        struct inotify_event *event = (struct inotify_event *) &buffer[ i ];
        handle_event(event);
        i += EVENT_SIZE + event->len;
      }
    }
  }

  // Close inotify instance (optional, process may be terminated differently)
  close(inotify_fd);

  return 0;
}
