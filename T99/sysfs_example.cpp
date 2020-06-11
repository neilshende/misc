//
// return cntlid of the nvme device
//
int
sysfsGetNvmeDeviceCntlid ( String nvme, short &cntlid )
{
    int error = -1;
    String CntlidPath = "/sys/block/" + nvme + "/device/cntlid";
    std::ifstream sysfs( CntlidPath.c_str() );
    if ( sysfs >> cntlid ) {
        error = 0; 
    }    
    return error;
}

