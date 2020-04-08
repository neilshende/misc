#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
//#include <unistd.h>
#include <errno.h>
//#include <string.h>
//#include <stdio.h>
#include <stdint.h>
#include "sgio.h"

#define String std::string
//#include "sgio.cpp"
#include "scsiApi.h"
void red (void) {
    printf("\033[1;31m");
}

void yellow (void) {
    printf("\033[1;33m");
}

void blue (void) {
    printf("\033[1;34m");
}

void green (void) {
    printf("\033[1;32m");
}

void black (void) {
    printf("\033[0m");
}
void (*pf[])(void) = {black, red, yellow, blue, green};

static void
signal_handler(int SignalNum)
{
    blue();
    printf("Interrupted.\n");
    black();
    exit(0);
}

char *PROGRAM;

void help(void) {
    pf[4]();
    printf("%s [acquire|release|change|reset|check] <path-of-nvme-device> <key>\n", PROGRAM);
    pf[0]();
    exit(0);
}
void error(int n) {
    pf[1]();
    printf("invalid option %d.\n", n);
    pf[0]();
    exit(n);
}

int main(int argc, char *argv[])
{
    struct sigaction sigact;
    int  err;
    int fi;
    uint64_t key;
    
    PROGRAM = argv[0];

    memset(&sigact, 0, sizeof (sigact));
    sigact.sa_handler = &signal_handler;
    err = sigaction(SIGTERM, &sigact, NULL);
    err = sigaction(SIGINT, &sigact, NULL);
    
    if (argc != 4) {
        help();
        exit(0);
    }
    
    if (strcmp(argv[1], "acquire") == 0) fi=0;
    else if (strcmp(argv[1], "release") == 0) fi=1;
    else if (strcmp(argv[1], "change") == 0) fi=2;
    else if (strcmp(argv[1], "reset") == 0) fi=3;
    else if (strcmp(argv[1], "check") == 0) fi=4;
    else error(1);

#if 0
    std::string nvme = argv[2]+5;
    std::string CntlidPath = "/sys/block/" + nvme + "/device/cntlid";
    std::ifstream sysfs( CntlidPath.c_str() );
    short cntlid;
    if ( sysfs >>cntlid ) {
        std::cout << "The cntlid is : " << cntlid << std::endl;
    } else {
        std::cout << "unable to read cntlid" << std::endl;
        exit(-1);
    }
#endif
    
    err = sscanf(argv[3], "%llx", &key);
    if (err != 1) error(3);
    switch(fi) {
	case 0: err = acquireReservation(argv[2], key);
	        break;
	case 1: err = releaseReservation(argv[2], key);
	        break;
	case 2: err = changeReservation(argv[2], key);
	        break;
	case 3: err = resetReservation(argv[2], key);
	        break;
	case 4: err = checkReservation(argv[2], key);
	        printf("The key is 0x%llx\n", key);
	        break;
	default: error(4);
    }
    printf("The error is %d \n", err);
    return err;
}
