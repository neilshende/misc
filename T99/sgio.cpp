// Copyright 2010, Tintri, Inc. All rights reserved.
//
// This file contains common functions related to SES operations.
//
//
#if 0
#include <sys/types.h>
//#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "sgio.h"
#endif
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
#include <stdio.h>
#include "sgio.h"

#define String std::string
#define ATA_IDENTIFY_MODEL_OFFSET 54
#define ATA_IDENTIFY_MODEL_LENGTH 40

//
// Function for open and close a SG device.
//
int
sgOpenDevice( const char *devname ) {
    int file = -1;

    while ( file == -1 ) {
        file = open( devname, O_RDWR);

        if ( file == -1 && errno != EINTR ) {
            break;
        }
    }

    return file;
}

int
sgCloseDevice( int file ) {
    int error = 0;

    while ( file != -1 ) {
        error = close( file );

        if ( ! error || errno != EINTR ) {
            if ( error == -1 ) {
                error = errno;
            }
            break;
        }
    }

    return error;
}

//
// Inline functions to initialize a SG_IO hearder.
//
static inline void
sgInitCdb( struct sg_io_hdr *sgio, uint8_t *cdb, size_t cmdlen )
{
    sgio->cmdp = cdb;
    sgio->cmd_len = cmdlen;
}

static inline void
sgInitDataNone( struct sg_io_hdr *sgio )
{
    sgio->iovec_count = 0;
    sgio->dxferp = 0;
    sgio->dxfer_len = 0;
    sgio->resid = 0;
    sgio->dxfer_direction = SG_DXFER_NONE;
}

static inline void
sgInitDataIn( struct sg_io_hdr *sgio, void *data, size_t size )
{
    sgio->iovec_count = 0;
    sgio->dxferp = data;
    sgio->dxfer_len = size;
    sgio->resid = 0;
    sgio->dxfer_direction = SG_DXFER_FROM_DEV;
}

static inline void
sgInitDataOut( struct sg_io_hdr *sgio, void *data, size_t size )
{
    sgio->iovec_count = 0;
    sgio->dxferp = data;
    sgio->dxfer_len = size;
    sgio->resid = 0;
    sgio->dxfer_direction = SG_DXFER_TO_DEV;
}

static inline void
sgInitSenseBuffer( struct sg_io_hdr *sgio, uint8_t *buffer, size_t length )
{
    sgio->sbp = buffer;
    sgio->mx_sb_len = length;
    memset( buffer, 0, length );
}

//
// Check sense data to determine command status.
//
static inline int
sgCheckCmdStatus( struct sg_io_hdr *sgio )
{
    int result = 0;

    if (sgio->status || sgio->host_status || sgio->driver_status) {
        result = -EIO;
        // Reservation conflict
        if (sgio->status == 0x18) {
            result = -EKEYREVOKED;
        }
    }

    if ( ! sgio->sbp || ! sgio->sb_len_wr ) {
        return result;
    }

    uint8_t sense_key = 0, asc = 0, ascq = 0;

    if ( ( sgio->sbp[0] == 0x70 || sgio->sbp[0] == 0x71 ) && sgio->sb_len_wr >= 14 ) {
        sense_key = sgio->sbp[2] & 0x0f;
        asc = sgio->sbp[12];
        ascq = sgio->sbp[13];
    }

    if ( ( sgio->sbp[0] == 0x72 || sgio->sbp[0] == 0x73 ) && sgio->sb_len_wr >= 4 ) {
        sense_key = sgio->sbp[1] & 0x0f;
        asc = sgio->sbp[2];
        ascq = sgio->sbp[3];
    }

    if ( sense_key == RECOVERED_ERROR ) {
        return 0;
    }

    if ( sense_key == UNIT_ATTENTION ) {
        // Power on reset or reservation cleared/preempted.
        if ( ( asc == 0x29 ) ||
             ( asc == 0x2a && ascq == 0x03 ) ||
             ( asc == 0x2a && ascq == 0x05 ) ) {
            return -EAGAIN;
        }
    }

    return result;
}

//
// Send a SCSI command through SG_IO.
//
static inline int
sgSendCommand( int fd, struct sg_io_hdr *sgio, int timeout, int retry )
{
    int result;

    sgio->interface_id = 'S';
    sgio->timeout = timeout * 1000;
    sgio->flags = SG_FLAG_DIRECT_IO;

    // If the drive we are sending this ioctl is locked,
    // we might end up getting -EAGAIN every time and never
    // exit this loop. So, try only some number of times
    // before giving up.
    do {
        retry--;
        result = ioctl( fd, SG_IO, sgio );
        if ( result ) {
            result = -errno;
        } else {
            result = sgCheckCmdStatus( sgio );
        }
    } while ( ( result == -EAGAIN ) && ( retry > 0 ) );

    return result;
}



//
// inline functions to initialize a SCSI CDB.
//
static inline void
sgInitScsiPR( uint8_t *buf, uint8_t opcode, uint8_t action, uint8_t type, uint32_t size )
{
    scsi_reserve_out_cdb_t *cdb = (scsi_reserve_out_cdb_t *)buf;

    memset( buf, 0, sizeof( *cdb ) );

    cdb->opcode = opcode;
    cdb->action = action;

    if ( opcode == SCSI_RESERVE_OUT ) {
        cdb->type = type;

        cdb->length[0] = size >> 24;
        cdb->length[1] = size >> 16;
    }

    cdb->length[2] = size >> 8;
    cdb->length[3] = size & 0xff;

    cdb->control = 0;
}

static inline void
satInitPassthru( uint8_t *cdb, uint8_t cmd, uint8_t feature, uint8_t proto, uint32_t lba, uint8_t sectors )
{
    cdb[0] = SAT_ATA_PASSTHRU_16;
    cdb[1] = proto << 1;
    switch ( proto ) {
    case ATA_PIO_DATA_IN:
        cdb[2] = sectors ? 0x0e : 0;
        break;
    case ATA_PIO_DATA_OUT:
        cdb[2] = sectors ? 0x06 : 0;
        break;
    default:
        cdb[2] = 0;
        break;
    }
    cdb[3] = 0;
    cdb[4] = feature;
    cdb[5] = 0;
    cdb[6] = sectors; // sector count
    cdb[7] = 0;
    cdb[8] = lba & 0xff;         // LBA low
    cdb[9] = 0;
    cdb[10] = (lba >> 8) & 0xff; // LBA mid
    cdb[11] = 0;
    cdb[12] = (lba >> 16) & 0xff; // LBA high
    cdb[13] = 0;  // device
    cdb[14] = cmd;
    cdb[15] = 0;  // control
}

static inline void
sesInitControlCdb( uint8_t *cdb, unsigned page, size_t maxout )
{
    cdb[0] = SEND_DIAGNOSTIC;
    cdb[1] = 0x10; // Self-test code, PF and offline bits.
    cdb[2] = 0; // Reserved.
    cdb[3] = (uint8_t)((maxout >> 8) & 0xff);
    cdb[4] = (uint8_t)(maxout & 0xff);
    cdb[5] = 0;
}

static inline void
sesInitStatusCdb( uint8_t *cdb, uint8_t page, size_t maxin )
{
    cdb[0] = RECEIVE_DIAGNOSTIC;
    cdb[1] = 0x01; // Page Code Valid (bit 0).
    cdb[2] = page; // Page code.
    cdb[3] = (uint8_t)((maxin >> 8) & 0xff);
    cdb[4] = (uint8_t)(maxin & 0xff);
    cdb[5] = 0;
}


//
// SES Receive Diagnostic page.
//
int
sesRecvDiagnostic( int fd, uint8_t page, void *data, size_t *size, int timeout )
{
    struct sg_io_hdr sgio;
    uint8_t cdb[SES_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];

    memset( &sgio, 0, sizeof( sgio ) );

    sesInitStatusCdb( cdb, page, *size );
    sgInitCdb( &sgio, cdb, sizeof(cdb) );

    sgInitDataIn( &sgio, data, *size );
    sgInitSenseBuffer( &sgio, sense, sizeof(sense) );

    int error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );
    if ( ! error ) {
        *size -= sgio.resid;
    }

    return error;
}

//
// SES Send Diagnostics page.
//
int
sesSendDiagnostic( int fd, uint8_t page, void *data, size_t *size, int timeout )
{
    struct sg_io_hdr sgio;
    uint8_t cdb[SES_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];

    memset( &sgio, 0, sizeof( sgio ) );

    sesInitStatusCdb( cdb, page, *size );
    sgInitCdb( &sgio, cdb, sizeof(cdb) );

    sgInitDataOut( &sgio, data, *size );
    sgInitSenseBuffer( &sgio, sense, sizeof(sense) );

    int error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );
    if ( ! error ) {
        *size -= sgio.resid;
    }

    return error;
}

//
// Find the Element Type Header of a specified type in a SES Configuration Page.
//
ses_type_header_t *
sesFindTypeHeader( ses_config_page_t *configPage, size_t size, uint8_t type )
{
    const ses_enclosure_desc_t *desc;
    ses_type_header_t *header = NULL;
    uint32_t length = 0, count = 0;

    // Skip all the Enclosure Descriptors.
    for ( uint8_t i = 0; ( i <= configPage->num_sub_encls ) && ( length < size ); i++ ) {
        desc = (const ses_enclosure_desc_t *)(configPage->page_data + length);

        length += 4 + desc->length;
        count += desc->num_type_headers;
    }

    while ( count && ( size - length ) >= sizeof( ses_type_header_t ) ) {
        header = (ses_type_header_t *)(configPage->page_data + length);

        if ( header->element_type == type ) {
            break;
        }

        length += sizeof( *header );
        count--;
    }
    
    return header;
}

//
// Return the pointer to the first Type Descriptor Header and the number of Type
// Descriptor Headers of a specified subenclosure in an SES Configuration Page.
//
// Note: the caller must make sure the Configuration Page contains all the data.
//
ses_type_header_t *
sesLocateTypeHeader( ses_config_page_t *configPage, int *typeCount )
{
    const ses_enclosure_desc_t *desc;
    uint32_t length = 0;
    int total = 0;

    for ( uint8_t i = 0; i <= configPage->num_sub_encls; i++ ) {
        desc = (const ses_enclosure_desc_t *)(configPage->page_data + length);

        length += 4 + desc->length;
        total += desc->num_type_headers;
    }

    *typeCount = total;
    return (ses_type_header_t *)(configPage->page_data + length);
}

//
// Return the pointer to the first element status and the number of elements
// of a specified element type in an SES Status Page.
//
// Note: the caller must make sure the Configuration Page contains all the data.
//
ses_status_t *
sesLocateStatusElements( ses_status_page_t *statusPage, ses_element_type_t type,
                         ses_type_header_t typeHeader[], int typeCount,
                         int *elemCount )
{
    ses_status_t *elem = 0;
    int total = 0;

    *elemCount = 0;

    for ( int i = 0; i < typeCount; i++ ) {
        if ( type == typeHeader[i].element_type ) {
            elem = (ses_status_t *)( statusPage->page_data + total * sizeof(ses_status_t) );
            *elemCount = typeHeader[i].num_elements;
            break;
        }
        total += 1 + typeHeader[i].num_elements;
    }

    return elem;
}

//
// Send a SCSI Persistent Reservation In of Read Status action command.
//
int
sgScsiPRInReadStatus( int fd, uint8_t *buffer, size_t size, int timeout )
{
    uint8_t cdb[SCSI_RESERVE_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    struct sg_io_hdr sgio;
    int error;

    memset( &sgio, 0, sizeof( sgio ) );
    memset( buffer, 0, size );

    sgInitScsiPR( cdb, SCSI_RESERVE_IN, SCSI_PR_READ_STATUS, 0, size );

    sgInitCdb( &sgio, cdb, sizeof( cdb ) );
    sgInitDataIn( &sgio, buffer, size );
    sgInitSenseBuffer( &sgio, sense, sizeof( sense ) );

    error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );

    return error;
}

//
// Send a SCSI Persistent Reservation Out of Register action command.
//
int
sgScsiPROutRegister( int fd, int ptpl, uint64_t rsvkey, uint64_t svckey, int timeout )
{
    uint8_t cdb[SCSI_RESERVE_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    scsi_pr_out_basic_t param;
    struct sg_io_hdr sgio;
    int error;

    memset( &sgio, 0, sizeof( sgio ) );
    memset( &param, 0, sizeof( param ) );

    sgInitScsiPR( cdb, SCSI_RESERVE_OUT, SCSI_PR_REGISTER, 0, sizeof( param ) );

    memcpy(param.reserve_key, &rsvkey, sizeof(rsvkey));
    memcpy(param.service_key, &svckey, sizeof(svckey));

    param.ptpl_enable = ptpl ? 1 : 0;

    sgInitCdb( &sgio, cdb, sizeof( cdb ) );
    sgInitDataOut( &sgio, (uint8_t *)&param, sizeof( param ) );
    sgInitSenseBuffer( &sgio, sense, sizeof( sense ) );

    error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );

    return error;
}

//
// Send a SCSI Persistent Reservation Out of Register and Ignore action command.
//
int
sgScsiPROutRegIgnore( int fd, int ptpl, uint64_t rsvkey, uint64_t svckey, int timeout )
{
    uint8_t cdb[SCSI_RESERVE_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    scsi_pr_out_basic_t param;
    struct sg_io_hdr sgio;
    int error;

    memset( &sgio, 0, sizeof( sgio ) );
    memset( &param, 0, sizeof( param ) );

    sgInitScsiPR( cdb, SCSI_RESERVE_OUT, SCSI_PR_REG_IGNR, 0, sizeof( param ) );

    memcpy(param.reserve_key, &rsvkey, sizeof(rsvkey));
    memcpy(param.service_key, &svckey, sizeof(svckey));

    param.ptpl_enable = ptpl ? 1 : 0;

    sgInitCdb( &sgio, cdb, sizeof( cdb ) );
    sgInitDataOut( &sgio, (uint8_t *)&param, sizeof( param ) );
    sgInitSenseBuffer( &sgio, sense, sizeof( sense ) );

    error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );

    return error;
}

//
// Send a SCSI Persistent Reservation Out of Reserve action command.
//
int
sgScsiPROutReserve( int fd, uint64_t rsvkey, uint8_t type, int timeout )
{
    uint8_t cdb[SCSI_RESERVE_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    scsi_pr_out_basic_t param;
    struct sg_io_hdr sgio;
    int error;

    memset( &sgio, 0, sizeof( sgio ) );
    memset( &param, 0, sizeof( param ) );

    sgInitScsiPR( cdb, SCSI_RESERVE_OUT, SCSI_PR_RESERVE, type, sizeof( param ) );

    memcpy(param.reserve_key, &rsvkey, sizeof(rsvkey));

    sgInitCdb( &sgio, cdb, sizeof( cdb ) );
    sgInitDataOut( &sgio, (uint8_t *)&param, sizeof( param ) );
    sgInitSenseBuffer( &sgio, sense, sizeof( sense ) );

    error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );

    return error;
}

//
// Send a SCSI Persistent Reservation Out of Release action command.
//
int
sgScsiPROutRelease( int fd, uint64_t rsvkey, uint8_t type, int timeout, int retry )
{
    uint8_t cdb[SCSI_RESERVE_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    scsi_pr_out_basic_t param;
    struct sg_io_hdr sgio;
    int error;

    memset( &sgio, 0, sizeof( sgio ) );
    memset( &param, 0, sizeof( param ) );

    sgInitScsiPR( cdb, SCSI_RESERVE_OUT, SCSI_PR_RELEASE, type, sizeof( param ) );

    memcpy(param.reserve_key, &rsvkey, sizeof(rsvkey));

    sgInitCdb( &sgio, cdb, sizeof( cdb ) );
    sgInitDataOut( &sgio, (uint8_t *)&param, sizeof( param ) );
    sgInitSenseBuffer( &sgio, sense, sizeof( sense ) );

    error = sgSendCommand( fd, &sgio, timeout, retry );

    return error;
}

//
// Send a SCSI Persistent Reservation Out of Clear action command.
//
int
sgScsiPROutClear( int fd, uint64_t rsvkey, int timeout )
{
    uint8_t cdb[SCSI_RESERVE_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    scsi_pr_out_basic_t param;
    struct sg_io_hdr sgio;
    int result;

    memset( &sgio, 0, sizeof( sgio ) );
    memset( &param, 0, sizeof( param ) );

    sgInitScsiPR( cdb, SCSI_RESERVE_OUT, SCSI_PR_CLEAR, 0, sizeof( param ) );

    memcpy(param.reserve_key, &rsvkey, sizeof(rsvkey));

    sgInitCdb( &sgio, cdb, sizeof( cdb ) );
    sgInitDataOut( &sgio, (uint8_t *)&param, sizeof( param ) );
    sgInitSenseBuffer( &sgio, sense, sizeof( sense ) );

    result = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );

    return result;
 }

//
// Send a SCSI Persistent Reservation Out of Reempt action command.
//
int
sgScsiPROutPreempt( int fd, uint64_t rsvkey, uint64_t svckey, uint8_t type, int timeout )
{
    uint8_t cdb[SCSI_RESERVE_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    scsi_pr_out_basic_t param;
    struct sg_io_hdr sgio;
    int error;

    memset( &sgio, 0, sizeof( sgio ) );
    memset( &param, 0, sizeof( param ) );

    sgInitScsiPR( cdb, SCSI_RESERVE_OUT, SCSI_PR_PREEMPT, type, sizeof( param ) );

    memcpy(param.reserve_key, &rsvkey, sizeof(rsvkey));
    memcpy(param.service_key, &svckey, sizeof(svckey));

    sgInitCdb( &sgio, cdb, sizeof( cdb ) );
    sgInitDataOut( &sgio, (uint8_t *)&param, sizeof( param ) );
    sgInitSenseBuffer( &sgio, sense, sizeof( sense ) );

    error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );

    return error;
}

//
// Send a SMART Enable/Disable Attribute Autosave command by SAT ATA Pass-Through.
//
int
satSmartAutosave( int fd, int enable, int timeout )
{
    uint8_t cdb[SAT_PASSTHRU_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    struct sg_io_hdr sgio;
    int error;

    memset( &sgio, 0, sizeof( sgio ) );

    if ( enable ) {
        satInitPassthru( cdb, ATA_SMART, SMART_AUTOSAVE, ATA_NON_DATA, 0xc24f00, 0xf1 );
    } else {
        satInitPassthru( cdb, ATA_SMART, SMART_AUTOSAVE, ATA_NON_DATA, 0xc24f00, 0 );
    }

    sgInitCdb( &sgio, cdb, sizeof( cdb ) );
    sgInitDataNone( &sgio );
    sgInitSenseBuffer( &sgio, sense, sizeof( sense ) );

    error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );

    return error;
}

//
// Send a SMART Enable/Disable Operations command by SAT ATA Pass-Through.
//
int
satSmartOperations( int fd, int enable, int timeout )
{
    uint8_t cdb[SAT_PASSTHRU_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    struct sg_io_hdr sgio;
    int error;

    memset( &sgio, 0, sizeof( sgio ) );

    if ( enable ) {
        satInitPassthru( cdb, ATA_SMART, SMART_ENABLE_OPS, ATA_NON_DATA, 0xc24f00, 0 );
    } else {
        satInitPassthru( cdb, ATA_SMART, SMART_DISABLE_OPS, ATA_NON_DATA, 0xc24f00, 0 );
    }

    sgInitCdb( &sgio, cdb, sizeof( cdb ) );
    sgInitDataNone( &sgio );
    sgInitSenseBuffer( &sgio, sense, sizeof( sense ) );

    error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );

    return error;
}

//
// Send a SMART Read Data command by SAT ATA Pass-Through.
//
int
satSmartReadData( int fd, uint8_t *dataBuff, size_t *dataSize, int timeout )
{
    uint8_t cdb[SAT_PASSTHRU_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    struct sg_io_hdr sgio;
    int error;

    memset( &sgio, 0, sizeof( sgio ) );

    satInitPassthru( cdb, ATA_SMART, SMART_READ_DATA, ATA_PIO_DATA_IN, 0xc24f00, 1 );

    sgInitCdb( &sgio, cdb, sizeof( cdb ) );
    sgInitDataIn( &sgio, dataBuff, *dataSize );
    sgInitSenseBuffer( &sgio, sense, sizeof( sense ) );

    error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );
    if ( ! error ) {
        *dataSize = *dataSize - sgio.resid;
    }

    return error;
}

//
// Send a SMART Read Threshold command by SAT ATA Pass-Through.
//
int
satSmartReadThresholds( int fd, uint8_t *dataBuff, size_t *dataSize, int timeout )
{
    uint8_t cdb[SAT_PASSTHRU_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    struct sg_io_hdr sgio;
    int error;

    memset( &sgio, 0, sizeof( sgio ) );

    satInitPassthru( cdb, ATA_SMART, SMART_READ_THRESHOLD, ATA_PIO_DATA_IN, 0xc24f01, 1 );

    sgInitCdb( &sgio, cdb, sizeof( cdb ) );
    sgInitDataIn( &sgio, dataBuff, *dataSize );
    sgInitSenseBuffer( &sgio, sense, sizeof( sense ) );

    error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );
    if ( ! error ) {
        *dataSize = *dataSize - sgio.resid;
    }

    return error;
}

//
// Send a SMART Enable/Disable Attribute Autosave command by SAT ATA Pass-Through.
//
int
standby_immediate( int fd, int timeout ) {
    uint8_t cdb[SAT_PASSTHRU_CDB_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    struct sg_io_hdr sgio;
    int error;
    
    memset( &sgio, 0, sizeof(sgio));
    satInitPassthru( cdb, ATA_STANDBY_IMMEDIATE, 0, ATA_NON_DATA, 0, 0);
    
    sgInitCdb( &sgio, cdb, sizeof(cdb));
    sgInitDataNone( &sgio );
    sgInitSenseBuffer( &sgio, sense, sizeof(sense));
    
    error = sgSendCommand( fd, &sgio, timeout, SG_DEFAULT_RETRY );
    return error;
}

int
flush_write_cache (int fd, int verbose) {
    uint8_t cdb[SYNC_CACHE_CMD_LEN] = {SYNCHRONIZE_CACHE, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense[SG_SENSE_BUF_SIZE];
    struct sg_io_hdr sgio;
    int error;
    
    memset( &sgio, 0, sizeof(sgio));
    
    sgInitCdb( &sgio, cdb, sizeof(cdb));
    sgInitDataNone( &sgio );
    sgInitSenseBuffer( &sgio, sense, sizeof(sense));
    
    error = sgSendCommand( fd, &sgio, SYNC_CACHE_TIME_OUT, SG_DEFAULT_RETRY );
    
    if (error != 0 && verbose) {
        fprintf(stderr, "flush_write_cache failed error:0x%x, status:0x%x, host_status:0x%x, driver_status:0x%x\n", 
            error, sgio.status, sgio.host_status, sgio.driver_status);
    }
    return error;
}

int
diskFwVer(const String &sgdev, String &rev) {
    String sgpath = "/dev/";
    struct sg_io_hdr sgio;
    uint8_t cdb[6] = {INQUIRY, 0, 0, 0, STD_INQUIRY_DATA_LEN, 0 };
    uint8_t data[STD_INQUIRY_DATA_LEN];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    char verStr[INQUIRY_REV_LEN+1];
    int result;
    int fd;

    sgpath += sgdev;
    
    fd = sgOpenDevice( sgpath.c_str() );
    if (fd == -1 ) {
        return 1;
    }

    memset(verStr, 0, sizeof(verStr));
    memset(&sgio, 0, sizeof(sgio));

    sgInitCdb(&sgio, cdb, sizeof(cdb));
    sgInitDataIn(&sgio, data, sizeof(data));
    sgInitSenseBuffer(&sgio, sense, sizeof(sense));

    result = sgSendCommand(fd, &sgio, SG_DEFAULT_TIMEOUT, SG_DEFAULT_RETRY);

    if ( !result) {
        int i;
        uint8_t *ver=&data[INQUIRY_REV_OFFSET];
        for (i=0; i<INQUIRY_REV_LEN; i++) {
            sprintf(verStr+i, "%c", *ver++);
    }
    rev = String( verStr );
    }
    (void)sgCloseDevice(fd);
    return result;
}

int
diskModel(const String &sgdev, char *model) {
    String sgpath = "/dev/";
    uint8_t cdb[SAT_PASSTHRU_CDB_LEN];
    struct sg_io_hdr sgio;
    uint8_t data[512] = { 0 }, *ver;
    uint8_t sense[SG_SENSE_BUF_SIZE] = {0};
    int error = 0, i, j, fd;

    sgpath += sgdev;

    fd = sgOpenDevice( sgpath.c_str() );
    if (fd == -1 ) {
        fprintf(stderr, "diskFwModel: sgOpenDevice failed for disk %s", sgpath.c_str());
        return 1;
    }

    memset(&sgio, 0, sizeof(sgio));

    /* feature is not applicable for ATA_IDENTIFY_DEVICE command */
    satInitPassthru( cdb, ATA_IDENTIFY_DEVICE, 0, ATA_PIO_DATA_IN, 0, 1 );
    sgInitCdb(&sgio, (uint8_t *)&cdb, sizeof(cdb));
    sgInitDataIn(&sgio, data, sizeof(data));
    sgInitSenseBuffer(&sgio, sense, sizeof(sense));

    error = sgSendCommand(fd, &sgio, SG_DEFAULT_TIMEOUT, SG_DEFAULT_RETRY);
    if ( 0 != error ) {
        if ( error != -EIO && error != -EKEYREVOKED ) {
            fprintf(stderr, "diskModel failed for disk %s error:0x%x, status:0x%x, host_status:0x%x, driver_status:0x%x\n",
                             sgpath.c_str(), error, sgio.status, sgio.host_status, sgio.driver_status);
        }
        (void)sgCloseDevice(fd);
        return (error);
    }

    ver = &data[ATA_IDENTIFY_MODEL_OFFSET];
    for ( i = 0; i < ATA_IDENTIFY_MODEL_LENGTH; i += 2) {
        for ( j = 1; j >= 0; j-- ) {
            if (*(ver+i+j) != ' ') {
                sprintf(model, "%c", *(ver+i+j));
                model++;
            }
        }
    }

    (void)sgCloseDevice(fd);
    return (error);
}

/*
 * Check if the disk has LSI or Marvell as the interposer
 * Returns:
 *        LSI_INTERPOSER
 *        MARVEL_INTERPOSER
 *        UNKNOWN_INTERPOSER
 */
interposer_type_e
disk_uses_interposer_type(int fd)
{
    struct sg_io_hdr sgio;
    uint8_t cdb[6] = {INQUIRY, 0x1, 0x89, 0, 0xfc, 0 };
    char data[256];
    uint8_t sense[SG_SENSE_BUF_SIZE];
    int result;

    memset( &sgio, 0, sizeof(sgio) );
    memset( data, 0, sizeof(data) );
    memset( sense, 0, sizeof(sense) );

    sgInitCdb( &sgio, cdb, sizeof(cdb) );
    sgInitDataIn( &sgio, ( uint8_t * )data, sizeof( data ) );
    sgInitSenseBuffer( &sgio, sense, sizeof ( sense ) );

    result = sgSendCommand( fd, &sgio, SG_DEFAULT_TIMEOUT, SG_DEFAULT_RETRY );

    if ( !result ) {
        if ( strncmp( &data[8], "LSI", strlen( "LSI" ) ) == 0 &&
            strncmp( &data[16], "LSISS25x0", strlen( "LSISS25x0" ) ) == 0) {
            return LSI_INTERPOSER;
        } else if ( strncmp(&data[8], "MARVELL", strlen( "MARVELL" ) ) == 0 ) {
            return MARVEL_INTERPOSER;
        } else {
            return MARVEL_INTERPOSER;
        }
    }

    return UNKNOWN_INTERPOSER;
}
