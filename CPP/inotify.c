#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

int main(int argc, char **argv) {
    int fd, wd;
    char buffer[EVENT_BUF_LEN];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Initialize inotify
    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    // Add a watch to the directory
    wd = inotify_add_watch(fd, argv[1],
                           IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF);
    if (wd < 0) {
        perror("inotify_add_watch");
        exit(EXIT_FAILURE);
    }

    printf("Watching %s\n", argv[1]);

    // Read events
    while (1) {
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            if (errno == EINTR) { // Interrupted by a signal
                continue;
            }
            perror("read");
            break;
        }

        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];
            if (event->len) {
                printf("File: %s\n", event->name);
            }

            if (event->mask & IN_CREATE) {
                if (event->isdir) {
                    printf("Directory created.\n");
                } else {
                    printf("File created.\n");
                }
            }
            if (event->mask & IN_DELETE) {
                if (event->isdir) {
                    printf("Directory deleted.\n");
                } else {
                    printf("File deleted.\n");
                }
            }
            if (event->mask & IN_MODIFY) {
                printf("File modified.\n");
            }
            if (event->mask & IN_MOVED_FROM) {
                printf("File moved from.\n");
            }
            if (event->mask & IN_MOVED_TO) {
                printf("File moved to.\n");
            }
            if(event->mask & IN_DELETE_SELF){
                printf("Watched directory deleted\n");
                return 0;
            }

            i += EVENT_SIZE + event->len;
        }
    }

    // Clean up (this is usually not reached due to the infinite loop)
    inotify_rm_watch(fd, wd);
    close(fd);

    return 0;
}
