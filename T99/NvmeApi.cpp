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
#include <string.h>
#include <endian.h>
#include <linux/types.h>
#include <stdbool.h>
#include <iostream>
#include <fstream>
//#include <fwk/String.h>
#include "NvmeApi.h"
#include "nvme.h"
#include <linux/nvme_ioctl.h>
//
// returns name space id of the fd.
//
static __u32
nvme_get_nsid( int fd )
{
    return ioctl( fd, NVME_IOCTL_ID );
}

//
// opens the device for sending nvme ioctls
// returns fd.
// on error fd is < 0.
//
static int
open_dev( const char *dev )
{
    int err, fd;
    struct stat nvme_stat;
    err = open( dev, O_RDONLY );
    if ( err < 0 ) {
        return err;
    }
    fd = err;
    err = fstat( fd, &nvme_stat );
    if ( err < 0 ) {
        close( fd );
        return err;
    }
    if ( !S_ISCHR( nvme_stat.st_mode ) && !S_ISBLK( nvme_stat.st_mode ) ) {
        close( fd );
        return -ENODEV;
    }
    return fd;
}

//
// helper function to report status of reservation
// returns:
//    0 if no controller has acquired reservation.
//      key unchanged.
//    1 if reservation is acquired by any controller,
//      key returned.
//
static int
nvme_resv_report( struct nvme_reservation_status *status, __u32 cdw11, uint64_t &key )
{
    /* test Extended Data Structure bit */
    if ( ( cdw11 & 0x1 ) == 0 ) {
        if ( status->rtype == 1 ) {
           key = le64toh( status->regctl_ds[0].rkey );
           return 1;
        }
    } else {
        struct nvme_reservation_status_ext *ext_status = ( struct nvme_reservation_status_ext * )status;
        if ( status->rtype == 1 ) {
            key = le64toh( ext_status->regctl_eds[0].rkey );
            return 1;
        }
    }
    return 0;
}

//
// helper function to register the key
// returns
//     0 on success
//     -1 on ioctl failure
//
// Note that we can register the same key that is acquired by other controller.
// (That's how reset is implemented.)
//
static int
reserve_register( int fd, __u32 nsid, uint64_t key ) {
    int err;
    __u8 rrega = 0; //register
    __u8 cptpl = 3; //reservations persist across power loss.
    bool iekey = 0; //don't ignore existing.
    __u64 crkey = 0;
    __u64 nrkey = key;
    __le64 payload[2] = { htole64( crkey ), htole64( nrkey ) };
    __u32 cdw10 = ( rrega & 0x7 ) | ( iekey ? 1 << 3 : 0 ) | cptpl << 30;
    struct nvme_passthru_cmd cmd;

    memset( &cmd, 0, sizeof( cmd ) );
    cmd.opcode = nvme_cmd_resv_register;
    cmd.nsid = nsid;
    cmd.cdw10 = cdw10;
    cmd.addr = ( __u64 ) ( payload );
    cmd.data_len    = sizeof( payload );
    cmd.timeout_ms  = NVME_TIMEOUT;

    err = ioctl( fd, NVME_IOCTL_IO_CMD, &cmd );
    if ( err ) {
        return -1;
    }
    return 0;
}

//
// helper function to unregister the key.
// returns
//     0 on success
//     -1 on ioctl failure
//
static int
reserve_unregister( int fd, __u32 nsid, uint64_t key ) {
    int err;
    __u8 rrega = 1; //unregister
    __u8 cptpl = 3; //reservations persist across power loss.
    bool iekey = 0; //don't ignore existing.
    __le64 payload[2] = { htole64( key ), htole64( 0 ) };
    __u32 cdw10 = ( rrega & 0x7 ) | ( iekey ? 1 << 3 : 0 ) | cptpl << 30;

    struct nvme_passthru_cmd cmd;
    memset( &cmd, 0, sizeof( cmd ) );
    cmd.opcode = nvme_cmd_resv_register;
    cmd.nsid = nsid;
    cmd.cdw10 = cdw10;
    cmd.addr = ( __u64 ) ( payload );
    cmd.data_len    = sizeof( payload );
    cmd.timeout_ms  = NVME_TIMEOUT;

    err = ioctl( fd, NVME_IOCTL_IO_CMD, &cmd );
    if ( err ) {
        return -1;
    }
    return 0;
}

//
// helper function to acquire Reservation with the specified key.
// returns
//     return value from ioctl
//
static int
reserve_acquire( int fd, __u32 nsid, uint64_t key ) {
    __u8 racqa = 0; //acquire.
    __u8 rtype = 1; //Write Exclusive reservation.
    bool iekey = 0; //don't ignore existing.

    __le64 payload[2] = { htole64( key ), htole64( 0 ) };
    __u32 cdw10 = ( racqa & 0x7 ) | ( iekey ? 1 << 3 : 0 ) | rtype << 8;
    struct nvme_passthru_cmd cmd;

    memset( &cmd, 0, sizeof( cmd ) );
    cmd.opcode = nvme_cmd_resv_acquire;
    cmd.nsid = nsid;
    cmd.cdw10 = cdw10;
    cmd.addr = ( __u64 ) ( payload );
    cmd.data_len    = sizeof( payload );
    cmd.timeout_ms  = NVME_TIMEOUT;

    return ioctl( fd, NVME_IOCTL_IO_CMD, &cmd );
}

//
// API function to acquire Reservation with the specified key.
// returns
//     0 on success
//     1 on reservation conflict.
//     -1 on ioctl failure.
//
//     corner cases:
//     1. trying to acquire reservation with another key when
//        already holding a reservation will cause ioctl failure error.
//     2. trying to acquire reservation with same key previously
//        acquired will succeed.
//     3. Trying to acquire, when other controller has acquired will
//        fail with err = 1.
//
int
NvmeAcquireHelper( const char *dev, uint64_t key ) {
    int fd = -1;
    int err, err2;
    __u32 nsid;

    fd = open_dev( dev );
    if ( fd < 0 ) {
        return -1;
    }
    nsid = nvme_get_nsid( fd );
    if ( nsid == 0 ) {
        close( fd );
        return -1;
    }

    err = reserve_register( fd, nsid, key );
    if ( err != 0 ) {
         close ( fd );
         return -1;
    }
    err = reserve_acquire( fd, nsid, key );
    if ( err ) {
       if ( err == NVME_SC_RESERVATION_CONFLICT ) {
           err = 1;
       } else {
           err = -1;
       }
       err2 = reserve_unregister( fd, nsid, key );
    }
    close( fd );
    return err;
}

//
// helper function to release the key.
// returns
//     0 on success
//     -1 on ioctl failure
//
static int
reserve_release( int fd, __u32 nsid, uint64_t key ) {
    __u8 rrela = 1; //clear
    __u8 rtype = 1; //Write exclusive
    bool iekey = 0;
    __u64 crkey = key;
    __le64 payload[1] = { htole64( crkey ) };
    __u32 cdw10 = ( rrela & 0x7 ) | ( iekey ? 1 << 3 : 0 ) | rtype << 8;
    struct nvme_passthru_cmd cmd;

    memset( &cmd, 0, sizeof( cmd ) );
    cmd.opcode = nvme_cmd_resv_release;
    cmd.nsid = nsid;
    cmd.cdw10 = cdw10;
    cmd.addr = ( __u64 ) ( payload );
    cmd.data_len    = sizeof( payload );
    cmd.timeout_ms  = NVME_TIMEOUT;

    return ioctl( fd, NVME_IOCTL_IO_CMD, &cmd );
}

//
// API function to release the key.
// returns
//     0 on success
//     -1 on ioctl failure
//     1 on reservation conflict
//
//    key should be the key acquired by this controller.
//    If key is incorrect, err -1 will be returned.
//    If reservation is acquired by the other controller, err 1 will be returned.
//
int
NvmeReleaseHelper( const char *dev,  uint64_t key ) {
    int fd = -1;
    int err;
    __u32 nsid;

    fd = open_dev( dev );
    if ( fd < 0 ) {
        return -1;
    }

    nsid = nvme_get_nsid( fd );
    if ( nsid == 0 ) {
      close( fd );
      return -1;
    }
    err = reserve_release( fd, nsid, key );
    if ( err ) {
       if ( err == NVME_SC_RESERVATION_CONFLICT ) {
          err = 1;
       } else {
           err = -1;
       }
    }

    close( fd );
    return err;
}

//
// API function to change the key.
// returns
//     0 on success
//     -1 on ioctl failure
//     1 on reservation conflict
//
//  corner case:
//     change to the same key acquired by this controller will succeed.
//
int
NvmeChangeHelper( const char *dev, uint64_t key ) {
    int fd = -1;
    int err;
    __u32 nsid;
    uint64_t ckey = 0;
    __u8 rrega = 2; //change register
    __u8 cptpl = 3; //reservations persist across power loss.
    bool iekey = 0; //don't ignore existing.
    __u64 crkey = ckey;
    __u64 nrkey = key;
    __le64 payload[2] = { htole64( crkey ), htole64( nrkey ) };
    __u32 cdw10 = ( rrega & 0x7 ) | ( iekey ? 1 << 3 : 0 ) | cptpl << 30;
    struct nvme_passthru_cmd cmd;

    err = NvmeCheckHelper( dev, ckey );
    if ( err == -1 ) {
        return err;
    }
    if ( ckey == 0 ) {
       // no one has acquired, so just acquire.
       err = NvmeAcquireHelper( dev, key );
       return err;
    }

    fd = open_dev( dev );
    if ( fd < 0 ) {
       return -1;
    }

    nsid = nvme_get_nsid( fd );
    if ( nsid == 0 ) {
       close( fd );
       return -1;
    }

    memset( &cmd, 0, sizeof( cmd ) );
    cmd.opcode = nvme_cmd_resv_register;
    cmd.nsid = nsid;
    cmd.cdw10 = cdw10;
    cmd.addr = ( __u64 ) ( payload );
    cmd.data_len    = sizeof( payload );
    cmd.timeout_ms  = NVME_TIMEOUT;

    err = ioctl( fd, NVME_IOCTL_IO_CMD, &cmd );
    if ( err ) {
	err = ( err == NVME_SC_RESERVATION_CONFLICT ) ? 1 : -1;
    }

    close( fd );
    return err;
}

//
// API function to reset the reservation.
//   Other controller may have acquired the reservation,
//   it will be released.
//
// returns
//     0 on success
//     -1 on ioctl failure
//
// Note that we reserve the same key acquired by the other controller.
//   when we release this key, the reservation is lost by the other controller.
// If the reservation is acquired by this controller, it works the same way.
//   we reserve the key and release it.
int
NvmeResetHelper( const char *dev, uint64_t key ) {
    int err1;
    int err2;
    int err3 = 0;
    uint64_t localkey = 0;
    int fd = -1;
    __u32 nsid;

    err1 = NvmeCheckHelper( dev, localkey );
    if ( err1 == -1 ) {
        return err1;
    }
    if ( localkey != 0 ) {
       fd = open_dev( dev );
       if ( fd < 0 ) {
           return -1;
       }

       nsid = nvme_get_nsid( fd );
       if ( nsid == 0 ) {
          close( fd );
          return -1;
       }
       err2 = reserve_register( fd, nsid, localkey );
       close( fd );
       if ( err2 == -1 ) {
          return -1;
       }

       err3 = NvmeReleaseHelper( dev, localkey );
    }
    return ( err3 == 0 ) ? 0 : -1;
}

//
// API function to check the reservation key.
// returns
//     0 if no controller has acquired reservation.
//        key unchanged.
//     -1 on ioctl failure.
//        key unchanged.
//     1 if any controller has acquired reservation.
//        key set to acquired key.
//
int
NvmeCheckHelper( const char *dev, uint64_t &key ) {
    int fd = -1;
    int err;
    __u32 nsid = 0;
    __u32 numd = 0x1000;
    __u32 cdw11 = 0;
    struct nvme_reservation_status *status;
    struct nvme_passthru_cmd cmd;

    fd = open_dev( dev );
    if ( fd < 0 ) {
        return -1;
    }

    nsid = nvme_get_nsid( fd );
    if ( nsid == 0 ) {
       close( fd );
       return -1;
    }

    if ( posix_memalign( ( void ** )&status, getpagesize(), ( numd + 1 ) * 4 ) ) {
       close( fd );
       return -1;
    }
    memset( status, 0, ( numd + 1 ) * 4 );

    memset( &cmd, 0, sizeof( cmd ) );
    cmd.opcode = nvme_cmd_resv_report;
    cmd.nsid = nsid;
    cmd.cdw10 = numd;
    cmd.cdw11 = cdw11;
    cmd.addr = ( __u64 )( unsigned long ) ( status );
    cmd.data_len    = ( numd + 1 ) * 4;
    cmd.timeout_ms  = NVME_TIMEOUT;


    err = ioctl( fd, NVME_IOCTL_IO_CMD, &cmd );
    if ( err == 0 ) {
        key = 0;
        err = nvme_resv_report( status, cdw11, key );
    } else {
        err = -1;
    }
    free( status );
    close( fd );
    return err;
}
