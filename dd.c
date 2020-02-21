#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int err=0;
    int bs=0, count=0;
    char *block=NULL;
    int nn=0, ii=0, pos=0;
    int fi=-1, fo=-1;

    if (argc != 5) {
        printf("Usage: %s <if> <of> <bs> <count> \n", argv[0]);
        printf("%s /dev/da0 /dev/null 4096 10\n", argv[0]);
        return 0;
    }
    fi = open(argv[1], O_RDONLY);
    if (fi < 0) {
        printf("error opening %s.\n", argv[1]);
        err = 1;
        goto bail;
    }
    fo = open(argv[2], O_RDWR|O_CREAT);
    if (fo < 0) {
        printf("error opening %s.\n", argv[2]);
        err = 2;
        goto bail;
    }
    sscanf(argv[3], "%d", &bs);
    if (bs==0) {
        printf("invalid bs %s.\n", argv[3]);
        err = 3;
        goto bail;
    }
    sscanf(argv[4], "%d", &count);
    if (count==0) {
        printf("invalid count %s.\n", argv[4]);
        err = 4;
        goto bail;
    }
    block=malloc(bs);
    if (block==NULL) {
        printf("out of mem.\n");
        err = 5;
        goto bail;
    }
    for (ii=0; ii<count; ii++, pos+=bs) {
        lseek(fi, pos, SEEK_SET);
        if ((nn = read(fi, block, bs)) != bs) {
            printf("error reading block %d.\n", ii);
            err = 6;
            goto bail;
        }
        write(fo, block, nn);
    }
bail:
    if (block != NULL) free(block);
    if (fi>0) close(fi);
    if (fo>0) close(fo);
    return err;
}

