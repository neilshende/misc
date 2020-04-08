#pragma once
#include <linux/types.h>

struct nvme_reservation_status {
    __le32 gen;
    __u8 rtype;
    __u8 regctl[2];
    __u8 resv5[2];
    __u8 ptpls;
    __u8 resv10[13];
    struct {
        __le16 cntlid;
        __u8 rcsts;
        __u8 resv3[5];
        __le64 hostid;
        __le64 rkey;
    } regctl_ds[];
};

struct nvme_reservation_status_ext {
    __le32 gen;
    __u8 rtype;
    __u8 regctl[2];
    __u8 resv5[2];
    __u8 ptpls;
    __u8 resv10[14];
    __u8 resv24[40];
    struct {
        __le16 cntlid;
        __u8 rcsts;
        __u8 resv3[5];
        __le64 rkey;
        __u8 hostid[16];
        __u8 resv32[32];
    } regctl_eds[];
};

enum nvme_opcode {
    nvme_cmd_resv_register = 0x0d,
    nvme_cmd_resv_report = 0x0e,
    nvme_cmd_resv_acquire = 0x11,
    nvme_cmd_resv_release = 0x15,
};

#define NVME_SC_RESERVATION_CONFLICT    0x83
#define NVME_TIMEOUT                    12000 // in milli-seconds.
