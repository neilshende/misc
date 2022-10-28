//
//  cli-test.c
//  
//
//  Created by Vivek Shende on 10/9/2019.
//

#include<stdio.h>
#include<string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "cli-test.h"

int check (long long *);

//global
char *DEVICE;
char *PROGRAM;

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

int get_nsid(int fd) 
{
        static struct stat nvme_stat;
        int err = fstat(fd, &nvme_stat);
        int nsid;

        if (err < 0)
                return -errno;

        if (!S_ISBLK(nvme_stat.st_mode)) {
                fprintf(stderr,
                        "Error: requesting namespace-id from non-block device\n");
                errno = ENOTBLK;
                return -errno;
        }   
        nsid = ioctl(fd, NVME_IOCTL_ID);
	printf("The nsid is %d\n", nsid);
	return nsid;
}


static int open_dev(char *dev)
{
        int err, fd;
        struct stat nvme_stat;
        err = open(dev, O_RDONLY);
        if (err < 0)
                goto perror;
        fd = err; 

        err = fstat(fd, &nvme_stat);
        if (err < 0) 
                goto perror;
        if (!S_ISCHR(nvme_stat.st_mode) && !S_ISBLK(nvme_stat.st_mode)) {
                fprintf(stderr, "%s is not a block or character device\n", dev);
                return -ENODEV;
        }    
        return fd;
perror:
        perror(dev);
        return err; 
}


void show_nvme_resv_report(struct nvme_reservation_status *status, int bytes, __u32 cdw11, long long *key, long long *keyx, short cntlid)
{
    int i, j, regctl, entries;
    
    regctl = status->regctl[0] | (status->regctl[1] << 8);
    
    printf("\nNVME Reservation status:\n\n");
    printf("gen       : %d\n", le32_to_cpu(status->gen));
    printf("rtype     : %d\n", status->rtype);
    printf("regctl    : %d\n", regctl);
    printf("ptpls     : %d\n", status->ptpls);
    
    /* check Extended Data Structure bit */
    if ((cdw11 & 0x1) == 0) {
        /* if status buffer was too small, don't loop past the end of the buffer */
        entries = (bytes - 24) / 24;
        if (entries < regctl)
            regctl = entries;
        if ((regctl > 0) && (status->rtype == 2)) *keyx = le64_to_cpu(status->regctl_ds[0].rkey);

        for (i = 0; i < regctl; i++) {
            printf("regctl[%d] :\n", i);
            printf("  cntlid  : %x\n", le16_to_cpu(status->regctl_ds[i].cntlid));
            printf("  rcsts   : %x\n", status->regctl_ds[i].rcsts);
            printf("  hostid  : %llx\n", le64_to_cpu(status->regctl_ds[i].hostid));
            printf("  rkey    : %llx\n", le64_to_cpu(status->regctl_ds[i].rkey));
            if ((status->rtype == 2)  && (cntlid == le16_to_cpu(status->regctl_ds[i].cntlid))) {
	            *key = le64_to_cpu(status->regctl_ds[i].rkey);
            }
        }
    } else {
        struct nvme_reservation_status_ext *ext_status = (struct nvme_reservation_status_ext *)status;
        /* if status buffer was too small, don't loop past the end of the buffer */
        entries = (bytes - 64) / 64;
        if (entries < regctl)
            regctl = entries;
        if ((regctl > 0) && (status->rtype == 2)) *keyx = le64_to_cpu(ext_status->regctl_eds[0].rkey);
        
        for (i = 0; i < regctl; i++) {
            printf("regctlext[%d] :\n", i);
            printf("  cntlid     : %x\n", le16_to_cpu(ext_status->regctl_eds[i].cntlid));
            printf("  rcsts      : %x\n", ext_status->regctl_eds[i].rcsts);
	        printf("  rkey       : %llx\n", le64_to_cpu(ext_status->regctl_eds[i].rkey));
            if ((status->rtype == 2) && (cntlid == le16_to_cpu(ext_status->regctl_eds[i].cntlid))) {
	            *key = le64_to_cpu(ext_status->regctl_eds[i].rkey);
            }
            printf("  hostid     : ");
            for (j = 0; j < 16; j++)
                printf("%x", ext_status->regctl_eds[i].hostid[j]);
            printf("\n");
        }
    }
    printf("\n");
}

int identify_controller(int fd, __u32 nsid, short *cntlid) {
    int err;
    struct nvme_id_ctrl ctrl;
    memset(&ctrl, 0, sizeof (struct nvme_id_ctrl));
    struct nvme_admin_cmd cmd = {
        .opcode = nvme_admin_identify,
        .nsid = 0,
        .addr = (__u64)(uintptr_t) &ctrl,
        .data_len = NVME_IDENTIFY_DATA_SIZE,
        .cdw10 = 1,
        .cdw11 = 0,
    };
    err = ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd);
    if (err == 0) {
        *cntlid = le16_to_cpu(ctrl.cntlid);
    } else {
        printf("error %d from ioctl.\n", err);
        err = -1;
    }
    return err;
}

int reserve_register(int fd, __u32 nsid, long long key) {
    int err;
    __u8 rrega = 0; //register
    __u8 cptpl = 3; //reservations persist across power loss.
    __u8 racqa = 0; //acquire.
    __u8 rtype = 2; //Exclusive reservation.
    bool iekey = 0; //don't ignore existing.
    __u64 crkey = 0;
    __u64 nrkey = key;
    __le64 payload[2] = { cpu_to_le64(crkey), cpu_to_le64(nrkey) };
    __u32 cdw10 = (rrega & 0x7) | (iekey ? 1 << 3 : 0) | cptpl << 30; 

    struct nvme_passthru_cmd cmd = {
        .opcode        = nvme_cmd_resv_register,
        .nsid        = nsid,
        .cdw10        = cdw10,
        .addr        = (__u64)(uintptr_t) (payload),
        .data_len    = sizeof(payload),
    };

    err = ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
    if (err) {
        printf("reserve error %d\n", err);
        return -1;
    }
    return 0;
}

int reserve_unregister(int fd, __u32 nsid, long long key) {
    int err;
    __u8 rrega = 1; //unregister
    __u8 cptpl = 3; //reservations persist across power loss.
    __u8 racqa = 0; //acquire.
    __u8 rtype = 2; //Exclusive reservation.
    bool iekey = 0; //don't ignore existing.
    __u64 crkey = 0;
    __u64 nrkey = key;
    __le64 payload[2] = { cpu_to_le64(key), cpu_to_le64(0) };
    __u32 cdw10 = (rrega & 0x7) | (iekey ? 1 << 3 : 0) | cptpl << 30;

    struct nvme_passthru_cmd cmd = {
        .opcode        = nvme_cmd_resv_register,
        .nsid        = nsid,
        .cdw10        = cdw10,
        .addr        = (__u64)(uintptr_t) (payload),
        .data_len    = sizeof(payload),
    };

    err = ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
    if (err) {
        printf("reserve error %d\n", err);
        return -1;
    }
    return 0;
}

int reserve_acquire(int fd, __u32 nsid, long long key) {
 
    __u8 cptpl = 3; //reservations persist across power loss.
    __u8 racqa = 0; //acquire.
    __u8 rtype = 2; //Exclusive reservation.
    bool iekey = 0; //don't ignore existing.
    __u64 crkey = 0;
    __u64 nrkey = key;

    __le64 payload[2] = { cpu_to_le64(key), cpu_to_le64(0) };
    __u32 cdw10 = (racqa & 0x7) | (iekey ? 1 << 3 : 0) | rtype << 8;
    struct nvme_passthru_cmd cmd = {
        .opcode        = nvme_cmd_resv_acquire,
        .nsid        = nsid,
        .cdw10        = cdw10,
        .addr        = (__u64)(uintptr_t) (payload),
        .data_len    = sizeof(payload),
    };
    
    return ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
}

int acquire(long long key) {
    int fd = -1;
    int err, err2;
    __u32 nsid;
#if 0
    __u8 rrega = 0; //register
    __u8 cptpl = 3; //reservations persist across power loss.
    __u8 racqa = 0; //acquire.
    __u8 rtype = 2; //Exclusive reservation.
    bool iekey = 0; //don't ignore existing.
    __u64 crkey = 0;
    __u64 nrkey = key;
#endif
    
    fd = open_dev(DEVICE);
    if (fd < 0) {
        printf("Unable to open %s\n", DEVICE);
        return -1;
    }
    nsid=get_nsid(fd);
    if (nsid == 0) {
	close(fd);
	printf("invalid nsid %d\n", nsid);
	return -1;
    }
   
   err = reserve_register(fd, nsid, key);
   if (err != 0) {
      return -1;
   }
#if 0
    __le64 payload[2] = { cpu_to_le64(key), cpu_to_le64(0) };
    __u32 cdw10 = (racqa & 0x7) | (iekey ? 1 << 3 : 0) | rtype << 8;
    struct nvme_passthru_cmd cmd = {
        .opcode        = nvme_cmd_resv_acquire,
        .nsid        = nsid,
        .cdw10        = cdw10,
        .addr        = (__u64)(uintptr_t) (payload),
        .data_len    = sizeof(payload),
    };
    
    err = ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
#endif
    err = reserve_acquire(fd, nsid, key);
    if (err) {
        printf("ioctl error %d\n", err);
	    if (err == 0x83) {
	       err = 1;
	    } else {
           err = -1;
	    }
        err2 = reserve_unregister(fd, nsid, key);
        if (err2 !=0) {
           printf("unable to unregister err(%d) key(0x%llx).\n", err2, key);
        }
    }
    close(fd);
    return err;
}

int reserve_release(int fd, __u32 nsid, long long key) {
#if 0
    __u8 rrega;
    __u8 cptpl;
#endif
    __u8 rrela = 1; //clear
    __u8 rtype = 2; //exclusive
    bool iekey = 0;
    __u64 crkey = key;
    __u64 nrkey = 0;

    __le64 payload[1] = { cpu_to_le64(crkey) };
    __u32 cdw10 = (rrela & 0x7) | (iekey ? 1 << 3 : 0) | rtype << 8;
    
    struct nvme_passthru_cmd cmd = {
        .opcode        = nvme_cmd_resv_release,
        .nsid        = nsid,
        .cdw10        = cdw10,
        .addr        = (__u64)(uintptr_t) (payload),
        .data_len    = sizeof(payload),
    };
    
    return ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
}

int release(long long key) {
    int fd = -1;
    int err;
    __u32 nsid;
    
#if 0
    __u8 rrega;
    __u8 cptpl;
    __u8 rrela = 1; //clear
    __u8 rtype = 2; //exclusive
    bool iekey = 0;
    __u64 crkey = key;
    __u64 nrkey = 0;
#endif

    fd = open_dev(DEVICE);
    if (fd < 0) {
        printf("Unable to open %s\n", DEVICE);
        return -1;
    }
    
    nsid=get_nsid(fd);
    if (nsid == 0) {
	   close(fd);
	   printf("invalid nsid %d\n", nsid);
	   return -1;
    }
#if 0
    __le64 payload[1] = { cpu_to_le64(crkey) };
    __u32 cdw10 = (rrela & 0x7) | (iekey ? 1 << 3 : 0) | rtype << 8;
    
    struct nvme_passthru_cmd cmd = {
        .opcode        = nvme_cmd_resv_release,
        .nsid        = nsid,
        .cdw10        = cdw10,
        .addr        = (__u64)(uintptr_t) (payload),
        .data_len    = sizeof(payload),
    };
    
    err = ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
#endif
    err = reserve_release(fd, nsid, key);
    if (err) {
        printf("ioctl error %d\n", err);
	    if (err == 0x83) {
	       err = 1;
	    } else {
           err = -1;
	    }
    }
    
    close(fd);
    return err;
}

int change (long long key) {
    int fd = -1;
    int err;
    __u32 nsid;

    fd = open_dev(DEVICE);
    if (fd < 0) {
        printf("Unable to open %s\n", DEVICE);
        return -1;
    }
    
    nsid=get_nsid(fd);
    if (nsid == 0) {
	close(fd);
	printf("invalid nsid %d\n", nsid);
	return -1;
    }

    long long ckey=0;
    err = check(&ckey);
    if (err != 0) {
        close(fd);
        return err==1 ? 1 : -1;
    }
    if (ckey == 0) {
       err = acquire(key);
       return err;
    }
 
    __u8 rrega = 2; //change register
    __u8 cptpl = 3; //reservations persist across power loss.
    __u8 racqa = 0; //acquire.
    __u8 rtype = 2; //Exclusive reservation.
    bool iekey = 0; //don't ignore existing.
    __u64 crkey = ckey;
    __u64 nrkey = key;
    __le64 payload[2] = { cpu_to_le64(crkey), cpu_to_le64(nrkey) };
    __u32 cdw10 = (rrega & 0x7) | (iekey ? 1 << 3 : 0) | cptpl << 30;

    struct nvme_passthru_cmd cmd = {
        .opcode        = nvme_cmd_resv_register,
        .nsid        = nsid,
        .cdw10        = cdw10,
        .addr        = (__u64)(uintptr_t) (payload),
        .data_len    = sizeof(payload),
    };

    err = ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
    if (err) {
        printf("ioctl error %d\n", err);
	    if (err == 0x83) {
	       err = 1;
	    } else {
           err = -1;
	    }
    }

    close(fd);
    return err;
}

int reset(long long key) {
    int err1, err2, err3=0;
    long long localkey = 0;
    int fd = -1;
    __u32 nsid;

    err1 = check(&localkey);
    if (localkey != 0) {
       fd = open_dev(DEVICE);
       if (fd < 0) {
           printf("Unable to open %s\n", DEVICE);
           return -1;
       }

       nsid=get_nsid(fd);
       if (nsid == 0) {
   	       close(fd);
	       printf("invalid nsid %d\n", nsid);
	       return -1;
       }
	   err2 = reserve_register(fd, nsid, localkey);
	   close(fd);

	   err3 = release(localkey);
    }
    if (err1 || err2 || err3) printf("ret codes of check acquire release %d %d %d.\n", err1, err2, err3);
    return err3==0? 0 : -1;
}

int report(long long key) {
    long long localkey = key;
    return check (&localkey);
}
int check(long long *pkey) {
    short cntlid;
    long long key = 0;
    int fd = -1;
    int err;
    __u32 nsid=0;
    __u8 rrega;
    __u8 cptpl;
    bool iekey;
    __u64 crkey;
    __u64 nrkey;
    __u32 numd=0x1000; //
    __u32 cdw11=0; //
    //char data[4096*8];
    struct nvme_reservation_status *status;
    
    //memset(data, 0, 4096*8);

    fd = open_dev(DEVICE);
    if (fd < 0) {
        printf("Unable to open %s\n", DEVICE);
        return -1;
    }

    nsid=get_nsid(fd);
    if (nsid == 0) {
	    close(fd);
        printf("invalid nsid %d\n", nsid);
	    return -1;
    }
    
    err = identify_controller(fd, nsid, &cntlid);
    if (err != 0) {
        close(fd);
        printf("error %d to get cntlid.\n", err);
        return -1;
    }
    
    if (posix_memalign((void **)&status, getpagesize(), 0x4004)) {
	    close(fd);
	    printf("memalign failed\n");
	    return -1;
    }
    memset(status, 0, 0x4004);
    struct nvme_passthru_cmd cmd = {
        .opcode        = nvme_cmd_resv_report,
        .nsid        = nsid,
        .cdw10        = numd,
        .cdw11        = cdw11,
        .addr        = (__u64)(uintptr_t) status,
	    .data_len    = (numd + 1) << 2,
    };
    
    err = ioctl(fd, NVME_IOCTL_IO_CMD, &cmd);
    if (err == 0) {
	    *pkey = 0;
	    show_nvme_resv_report(status, 0x4004, cdw11, pkey, &key, cntlid);

        // find out if this is our or peer's reservation.
        if (*pkey != 0) {
            err = reserve_acquire(fd, nsid, *pkey);
            if (err != 0) {
                err = (err==0x83) ? 1 : -1;
            }
        } else if (key != 0 ){
            *pkey = key;
            err = 1;
        }
        printf("The reservation key is 0x%llx\n", *pkey);

    } else {
	    printf("ioctl failed with error %d.\n", err);
	    err = -1;
    }
    close(fd);
    return err;
}

int (*pPRF[])(long long) = {acquire, release, change, reset, report};

static void
signal_handler(int SignalNum)
{
    blue();
    printf("The answer is 42. Thanks for all the fish.\n");
    black();
    exit(0);
}

void help(void) {
    pf[4]();
    printf("%s [acquire|release|change|reset|report] <path-of-nvme-device> <key>\n", PROGRAM);
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
    long long key;
    
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
    else if (strcmp(argv[1], "report") == 0) fi=4;
    else error(1);

    DEVICE = argv[2];
    
    err = sscanf(argv[3], "%llx", &key);
    if (err != 1) error(3);
    
    err = pPRF[fi](key);
    printf("return code is %d\n", err);
    
    return err;
}
