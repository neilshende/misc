// Copyright 2010, Tintri, Inc. All rights reserved.
//
// This file constains SG_IO related structure defintions and function prototypes.
//
#ifndef _PLATFORM_SGIO_H_
#define _PLATFORM_SGIO_H_

#include <stdint.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>
//#include <fwk/String.h>

#ifdef SG_DEFAULT_TIMEOUT
#undef SG_DEFAULT_TIMEOUT
#endif
#define SG_DEFAULT_TIMEOUT 10
#define SG_DEFAULT_RETRY   5

//
// Common open and close function prototypes.
//
int sgOpenDevice( const char *devname );
int sgCloseDevice( int file );

#define SG_SENSE_BUF_SIZE  32  // bytes

//
// SCSI command codes (not defined in scsi.h):
//
#define SCSI_RESERVE_IN      0x5e
#define SCSI_RESERVE_OUT     0x5f

#define SAT_ATA_PASSTHRU_12  0xa1
#define SAT_ATA_PASSTHRU_16  0x85

#define SAT_PASSTHRU_CDB_LEN 16

#define SYNC_CACHE_CMD_LEN   10
#define SYNC_CACHE_TIME_OUT  30  // time out in seconds

//
// SCSI Inquiry format
// 
#define INQUIRY_REV_LEN       4
#define STD_INQUIRY_DATA_LEN 36
#define INQUIRY_REV_OFFSET   32

//
// SAT ATA Pass-Through protocol field codes:
//
#define ATA_DM_HARD_RESET    0x0
#define ATA_DM_SOFT_RESET    0x1
#define ATA_NON_DATA         0x3
#define ATA_PIO_DATA_IN      0x4
#define ATA_PIO_DATA_OUT     0x5
#define ATA_DMA              0x6
#define ATA_DMA_QUEUED       0x7
#define ATA_EXEC_DIAG        0x8
#define ATA_NON_DATA_RESET   0x9
#define ATA_UDMA_DATA_IN     0xa
#define ATA_UDMA_DATA_OUT    0xb
#define ATA_FPDMA            0xc

//
// SES command timeout value.
//
#define SG_SES_TIMEOUT  SG_DEFAULT_TIMEOUT

//
// SES related definitions.
//
#define SES_CDB_LEN          6

#define SES_LOGICAL_ID_LEN   8   // in bytes
#define SES_VENDOR_ID_LEN    8   // in bytes
#define SES_PRODUCT_ID_LEN   16  // in bytes
#define SES_PRODUCT_REV_LEN  4   // in bytes

#define SES_DEFAULT_PAGE_SIZE 512 // in bytes

//
// SES element types:
//
typedef enum {
    SES_TYPE_UNSPECIFIED   = 0,
    SES_TYPE_DEVICE_SLOT   = 1,
    SES_TYPE_POWERSUPPLY   = 2,
    SES_TYPE_COOLING       = 3,
    SES_TYPE_TEMPERATURE   = 4,
    SES_TYPE_CONTROLLER    = 7,
    SES_TYPE_SSC_CONTROLER = 8,
    SES_TYPE_DISPLAY       = 12,
    SES_TYPE_KEYPAD        = 13,
    SES_TYPE_ENCLOSURE     = 14,
    SES_TYPE_VOLTAGESENSOR = 18,
    SES_TYPE_CURRENTSENSOR = 19,
    SES_TYPE_ARRAY_DEVICE  = 23
} ses_element_type_t;

//
// SES element status code:
//
enum {
    SES_STATUS_UNSUPPORTED  = 0,
    SES_STATUS_NORMAL       = 1,
    SES_STATUS_CRITICAL     = 2,
    SES_STATUS_NONCRITICAL  = 3,
    SES_STATUS_UNRECOVERABLE = 4,
    SES_STATUS_NOTINSTALLED  = 5,
    SES_STATUS_UNKNOWN       = 6,
    SES_STATUS_UNAVAILABLE   = 7,
    SES_STATUS_ACCESSNOTALLOWED = 8
};


//
// SES Configuration Page (1):
//
typedef struct {
    uint8_t page_code;      // Must be 1.
    uint8_t num_sub_encls;  // Number of secondary subenclosures.
    uint8_t page_length[2]; // Page length (MSB -> LSB) minus 4-byte header.
    uint8_t generation[4];  // Generation code (MSB -> LSB).
    uint8_t page_data[0];   // Enclosure descriptor list + Type header list + Type text list.
} __attribute__((packed)) ses_config_page_t;

//
// SES enclosure descriptor (config page data):
//
typedef struct {
    uint8_t num_process : 3;
    uint8_t reserved1   : 1;
    uint8_t process_id  : 3;
    uint8_t reserved2   : 1;
    uint8_t sub_encl_id;
    uint8_t num_type_headers;
    uint8_t length;
    uint8_t logical_id[SES_LOGICAL_ID_LEN];
    uint8_t vendor_id[SES_VENDOR_ID_LEN];
    uint8_t product_id[SES_PRODUCT_ID_LEN];
    uint8_t revision[SES_PRODUCT_REV_LEN];
    uint8_t specific[0];
} __attribute__((packed)) ses_enclosure_desc_t;

//
// SES type descriptor header (config page data):
//
typedef struct {
    uint8_t element_type;
    uint8_t num_elements;
    uint8_t sub_encl_id;
    uint8_t length;
} __attribute__((packed)) ses_type_header_t;


//
// SES Control Page (2):
//
typedef struct {
    uint8_t page_code;         // Must be 2.
    uint8_t unrecoverable : 1;
    uint8_t critical      : 1;
    uint8_t noncritical   : 1;
    uint8_t informational : 1;
    uint8_t reserved      : 4;
    uint8_t page_length[2];    // Page length (MSB -> LSB).
    uint8_t generation[4];     // Generation code (MSB -> LSB).
    uint8_t page_data[0];      // Overall controll + a list of individual element controls
} __attribute__((packed)) ses_control_page_t;

//
// SES Status Page (2):
//
typedef struct {
    uint8_t page_code;         // Must be 2.
    uint8_t unrecoverable : 1;
    uint8_t critical      : 1;
    uint8_t noncritical   : 1;
    uint8_t informational : 1;
    uint8_t invalid_op    : 1;
    uint8_t reserved      : 3;
    uint8_t page_length[2];     // Page length (MSB -> LSB) minus 4-byte header.
    uint8_t generation[4];      // Generation code (MSB -> LSB).
    uint8_t page_data[0];       // Overall status + a list of individual element statuses
} __attribute__((packed)) ses_status_page_t;

//
// SES Element Format (Control or Status Page data):
//
typedef struct {
    uint8_t reserved  : 4;
    uint8_t swapReset : 1;
    uint8_t disable   : 1;
    uint8_t failure   : 1;
    uint8_t select    : 1;
    uint8_t specific[3];
} __attribute__((packed)) ses_control_t;

//
// SES Element Format (Control or Status Page data):
//
typedef struct {
    uint8_t status   : 4;
    uint8_t swapped  : 1;
    uint8_t disabled : 1;
    uint8_t failure  : 1;
    uint8_t reserved : 1;
    uint8_t specific[3];
} __attribute__((packed)) ses_status_t;


//
// SES String in (page 4)
//
typedef struct {
    uint8_t page_code;         // Must be 4.
    uint8_t obsolete;
    uint8_t page_length[2];    // Page length (MSB -> LSB).
    uint8_t page_data[0];      // Vendor defined
} __attribute__((packed)) ses_stringin_page_t;


int sesRecvDiagnostic( int fd, uint8_t page, void *data, size_t *size, int timeout );
int sesSendDiagnostic( int fd, uint8_t page, void *data, size_t *size, int timeout );

ses_type_header_t *sesFindTypeHeader( ses_config_page_t *page, size_t size, uint8_t type );
ses_type_header_t *sesLocateTypeHeader( ses_config_page_t *page, int *count );

ses_status_t *sesLocateStatusElements( ses_status_page_t *page, ses_element_type_t type,
                                       ses_type_header_t header[], int count,
                                       int *nelements );

//
// SCSI persistent reservation related definitions.
//
#define SCSI_RESERVE_CDB_LEN 10

//
// Persistent reservation types.
//
#define SCSI_PR_WRITE_EX   1 // Write exclusive
#define SCSI_PR_ACCESS_EX  3 // Exclusive access
#define SCSI_PR_REG_WR_EX  5 // Registrants write exclusive
#define SCSI_PR_REG_ACC_EX 6 // Registrants exclusive access
#define SCSI_PR_ALL_WR_EX  7 // All registrants write exclusive
#define SCSI_PR_ALL_ACC_EX 8 // All registrants exclusive access

//
// Persistent reservation scopes.
//
#define SCSI_PR_SCOPE_LU   0 // Full logical unit

//
// Persistent Reserve In service action codes.
//
#define SCSI_PR_READ_KEYS   0
#define SCSI_PR_READ_RSRV   1
#define SCSI_PR_REPORT_CAP  2
#define SCSI_PR_READ_STATUS 3

//
// Persistent Reserve In CDB format.
//
typedef struct {
    uint8_t  opcode;     // SCSI_RESERVE_IN (0x5e)
    uint8_t  action : 5; // Service action.
    uint8_t  rsvd1 : 3;  // Reserved.
    uint8_t  rsvd2[5];   // Reserved.
    uint8_t  length[2];  // Allocation length; [0] - MSB, [1] - LSB.
    uint8_t  control;    // Control field.
} __attribute__((packed)) scsi_reserve_in_cdb_t;

//
// Read Keys service action paramter data format.
//
typedef struct {
    uint8_t  generation[4]; // Persistent reservation generation (MSB -> LSB).
    uint8_t  length[4];     // Additional length in bytes of the reservation key list that follows.
    uint8_t  key_list[8];   // List of reservation key, 8-byte each (MSB -> LSB).
} __attribute__((packed)) scsi_pr_read_keys_t;

//
// Read Reservation service action parameter data format.
//
typedef struct {
    uint8_t  generation[4]; // Persistent reservation generation (MSB -> LSB).
    uint8_t  length[4];     // Additional length; must be 16.
    uint8_t  key[8];        // Reservation key (MSB -> LSB).
    uint8_t  obsolete1[4];  // Obsolete field.
    uint8_t  reserved;      // Reserved field.
    uint8_t  type : 4;      // Reservation type.
    uint8_t  scope : 4;     // Reservation scope.
    uint8_t  obsolete2[2];  // Obsolete field.
} __attribute__((packed)) scsi_pr_read_rsrv_t;

//
// Report Capabilities service action parameter data format.
//
typedef struct {
    uint8_t  length[2];        // Much be 8.
    uint8_t  ptpl_capable : 1; // Persist through power loss capable.
    uint8_t  reserved1 : 1;
    uint8_t  atp_capable : 1;  // All target ports capable.
    uint8_t  sip_capable : 1;  // Specify initiator port capable.
    uint8_t  compatible : 1;   // SPC-2 Reserve/Release compatible.
    uint8_t  reserved2 : 3;
    uint8_t  ptpl_enabled : 1; // Persist thrugh power loss enabled.
    uint8_t  reserved3 : 3;
    uint8_t  allow_cmds : 3;   // Allow commands.
    uint8_t  type_valid : 1;   // Type mask valid; 1 indicates mask present.
    uint8_t  type_bitmap[2];   // Type mask bitmap.
    uint8_t  reserved[2];
} __attribute__((packed)) scsi_pr_report_cap_t;

//
// Read Full Status service action parameter data format.
//
typedef struct {
    uint8_t  generation[4]; // Persistent reservation generation (MSB -> LSB).
    uint8_t  length[4];     // Additional length in bytes (MSB -> LSB) of the descriptor list.
    uint8_t  desc_list[0];  // Full status descriptor list.
} __attribute__((packed)) scsi_pr_read_status_t;

//
// Full Status Descriptor format.
//
typedef struct {
    uint8_t  key[8];        // Reservation key.
    uint8_t  reserved1[4];
    uint8_t  holder : 1;    // Reservation holder's entry.
    uint8_t  alltgt : 1;    // All target ports; 0 - single I_T nexus, 1 - all I_T nexuses.
    uint8_t  rsvd : 6;
    uint8_t  type : 4;      // Reservation type.
    uint8_t  scope : 4;     // Reservation scope.
    uint8_t  reserved2[4];
    uint8_t  target_pid[2]; // Relative target port ID (MSB -> LSB).
    uint8_t  length[4];     // Additinal length in bytes (MSB -> LSB).
    uint8_t  transport[0];  // Transport ID identifying the initiator port.
} __attribute__((packed)) scsi_pr_status_desc_t;


//
// SCSI Persistent Reserve Out service action codes.
//
#define SCSI_PR_REGISTER  0
#define SCSI_PR_RESERVE   1
#define SCSI_PR_RELEASE   2
#define SCSI_PR_CLEAR     3
#define SCSI_PR_PREEMPT   4
#define SCSI_PR_PRMPT_ABT 5
#define SCSI_PR_REG_IGNR  6
#define SCSI_PR_REG_MOVE  7

//
// SCSI Persistent Reserve Out CDB format.
//
typedef struct {
    uint8_t  opcode;      // SCSI_RESERVE_OUT (0x5f)
    uint8_t  action : 5;  // Service action.
    uint8_t  rsvd : 3;
    uint8_t  type : 4;    // Reservation type.
    uint8_t  scope : 4;   // Reservation scope.
    uint8_t  reserved[2];
    uint8_t  length[4];   // Parameter list length in bytes.
    uint8_t  control;
} __attribute__((packed)) scsi_reserve_out_cdb_t;

//
// SCSI Persistent Reserve Out basic parameter list format.
//
typedef struct {
    uint8_t  reserve_key[8];  // Reservation key.
    uint8_t  service_key[8];  // Service action reversation key.
    uint8_t  obsolete1[4];
    uint8_t  ptpl_enable : 1; // Activate persitent through power loss.
    uint8_t  rsvd1 : 1;
    uint8_t  all_targets : 1; // All target ports (register).
    uint8_t  ini_specify : 1; // Specify initiator port.
    uint8_t  rsvd2 : 4;
    uint8_t  reserved;
    uint8_t  obsolete2[2];
    uint8_t  data[0];         // Additional parameter data.
} __attribute__((packed)) scsi_pr_out_basic_t;

int sgScsiPRInReadStatus( int fd, uint8_t *buffer, size_t size, int timeout );

int sgScsiPROutRegister( int fd, int ptpl, uint64_t rsvkey, uint64_t svckey, int timeout );
int sgScsiPROutRegIgnore( int fd, int ptpl, uint64_t rsvkey, uint64_t svckey, int timeout );
int sgScsiPROutReserve( int fd, uint64_t rsvkey, uint8_t type, int timeout );
int sgScsiPROutRelease( int fd, uint64_t rsvkey, uint8_t type, int timeout, int retry );
int sgScsiPROutClear( int fd, uint64_t rsvkey, int timeout );
int sgScsiPROutPreempt( int fd, uint64_t rsvkey, uint64_t svckey, uint8_t type, int timeout );


//
// ATA command codes:
//
#define ATA_SMART                0xb0
#define ATA_STANDBY_IMMEDIATE    0xe0
#define ATA_IDENTIFY_DEVICE      0xec

//
// ATA featue codes:
//
#define SMART_READ_DATA      0xd0
#define SMART_READ_THRESHOLD 0xd1 // Obsolete
#define SMART_AUTOSAVE       0xd2
#define SMART_EXEC_OFFLINE   0xd4
#define SMART_READ_LOG       0xd5
#define SMART_WRITE_LOG      0xd6
#define SMART_ENABLE_OPS     0xd8
#define SMART_DISABLE_OPS    0xd9
#define SMART_RET_STATUS     0xda

#define HDIO_GET_IDENTITY    0x030d
#define ATA_ID_FW_REV_BYTE   46
#define ATA_ID_FW_REV_LEN    8

//
// SAT command timeout value.
//
#define SG_SAT_TIMEOUT      SG_DEFAULT_TIMEOUT

int satSmartAutosave( int fd, int enable, int timeout );
int satSmartOperations( int fd, int enable, int timeout );
int satSmartReadData( int fd, uint8_t *dataBuff, size_t *dataSize, int timeout );
int satSmartReadThresholds( int fd, uint8_t *dataBuff, size_t *dataSize, int timeout );

// 
// flush the write cache of the SCSI block device that fd refers to.  returns 0 on success, non-zero on failure.
//
int flush_write_cache (int fd, int verbose);
int standby_immediate( int fd, int timeout );

//int diskFwVer( const String &sgdev, String &rev);
//int diskModel(const String &sgdev, char *model);

//
// Interposer types using in disks
//
enum interposer_type_e {
    LSI_INTERPOSER       = 1,
    MARVEL_INTERPOSER    = 2,
    UNKNOWN_INTERPOSER   = 3,
};

interposer_type_e disk_uses_interposer_type(int fd);

#endif // _PLATFORM_SGIO_H_
