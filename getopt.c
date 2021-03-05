#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <fcntl.h>
void usage() {
  printf("USAGE\n");
}
int
main (int argc, char *argv[])
{
     int bflag, ch, fd;
     int daggerset;

     /* options descriptor */
     static struct option longopts[] = {
         { "buffy", no_argument, NULL, 'b' },
         { "fluoride", required_argument, NULL, 'f' },
         /*{ "daggerset", no_argument, &daggerset, 1 },*/
         { "daggerset", no_argument, NULL, 1 },
         { "zset", required_argument, NULL, 'z' },
         { "xset", optional_argument, NULL, 'x' },
         { NULL, 0, NULL, 0 }
     };

     bflag = 0;
     while ((ch = getopt_long(argc, argv, "bf:x::yz:", longopts, NULL)) != -1) {
         switch (ch) {
             case 'x':
                 printf("x set %s\n", optarg);
                 break;
             case 'y':
                 printf("y set\n");
                 break;
             case 'z':
                 printf("z set %s\n", optarg);
                 break;

             case 'b':
                 bflag = 1;
                 printf("bflag set.\n");
                 break;
             case 'f':
                 if    ((fd = open(optarg, O_RDONLY, 0)) == -1)
                     printf("unable to open %s\n", optarg);
                 break;
             case 1:
                 if (1) {
                     printf("Buffy will use her dagger to "
                     "apply fluoride to dracula's teeth\n");
                 }
                 break;
             default:
                 usage();
         }
     }
     argc -= optind;
     argv += optind;
     return 0;
}
