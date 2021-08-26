#!/bin/bash
#
# Script allows to take EBS snapshot during CRIU dump cmd and properly
# mount this EBS snapshot on CRIU restore cmd.
#
#
# You should call this script on both CRIU dump and restore cmds.
#
# Usage:
# criu dump <options> --action-script '$CRIU_SCRIPTS_PATH/ebs_snap.sh volume-id mount-pt'
# criu restore <options> --action-script '$CRIU_SCRIPTS_PATH/ebs_snap.sh'
#
# Note: absolute path to ebs_snap.sh should be supplied in --action-script with
#
function findunused() {
  local f
  local g
  local found
  local -a devlist
  devlist=$(set -o pipefail; aws ec2 describe-volumes | awk '/Device/{print $2}' | tr -d ,\" | sort | uniq)
  if [ $? != 0 ]; then
     return 2
  fi
  for f in /dev/sd{f..p} ;do
    found="true"
    for g in ${devlist[@]} ;do
      if [ "$f" == "$g" ]; then
        found="false"
        break
      fi
    done
    if [ $found == "true" ]; then
      echo $f
      return 0
    fi
  done
  return 1
}

function findnewdev() {
   local found
   local odev
   local ndev
   local -a oldlist="$@"
   local -a newlist=$(set -o pipefail; nvme list 2>/dev/null | awk '/^\/dev\//{print $1}')
   if [ $? != 0 ]; then
      return 1
   fi
   for ndev in ${newlist[@]} ;do
      found="true"
      for odev in ${oldlist[@]} ;do
         if [ $ndev == $odev ]; then
            found="false"
            break
         fi
      done
      if [ $found == "true" ]; then
         echo $ndev
         return 0
      fi
   done
   return 1
}

function findnewdev_v2() {
   local found
   local odev
   local ndev
   local -a oldlist="$@"
   local -a newlist=$(set -o pipefail; lsblk --output NAME | grep ^xv)
   if [ $? != 0 ]; then
      return 1
   fi
   for ndev in ${newlist[@]} ;do
      found="true"
      for odev in ${oldlist[@]} ;do
         if [ $ndev == $odev ]; then
            found="false"
            break
         fi
      done
      if [ $found == "true" ]; then
         echo /dev/$ndev
         return 0
      fi
   done
   return 1
}


POSTDUMP="post-dump"
PRERESTORE="pre-restore"

MY_NAME=$(basename "$0")
case "$CRTOOLS_SCRIPT_ACTION" in
    $POSTDUMP )
        if [ "$#" -lt 2 ]; then
            echo "$MY_NAME $(date): ERROR! Missing argument."
            exit 1
        fi
        volid=$1
        mountpt=$2
        (set -o pipefail;
         curl http://169.254.169.254/latest/dynamic/instance-identity/document \
         2>/dev/null | tr -d {},\" > ebs_snap.info
        )
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): Failed to get instance-identity document."
           exit $rc
        fi
        volumetype=$(set -o pipefail; aws ec2 describe-volumes --volume-ids $volid | awk '/VolumeType/{print $2}' | tr -d ,\")
        rc=$?
        if [ $rc != 0 ] || [ -z "$volumetype" ]; then
           echo "$MY_NAME $(date): unable to find volume type of $volid."
           exit 1
        fi
        echo " VolumeType : " $volumetype >> ebs_snap.info
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): Failed to update ebs_snap.info with VolumeType : $volumetype."
           exit $rc
        fi

        iops=$(set -o pipefail; aws ec2 describe-volumes --volume-ids $volid | awk '/Iops/{print $2}' | tr -d ,\")
        rc=$?
        if [ $rc != 0 ] || [ -z "$iops" ]; then
           echo "$MY_NAME $(date): unable to find volume iops of $volid."
           exit 1
        fi
        echo " Iops : " $iops >> ebs_snap.info
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): Failed to update ebs_snap.info with Iops : $iops."
           exit $rc
        fi

        /bin/timeout 10 sync
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to sync in 10 seconds."
           exit $rc
        fi
        /bin/timeout 10 echo 3 >  /proc/sys/vm/drop_caches
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to drop cache in 10 seconds."
           exit $rc
        fi
        echo "$MY_NAME $(date): taking snapshot of $volid."
        snapid=$(set -o pipefail; aws ec2 create-snapshot --volume-id $volid | awk '/SnapshotId/{print $2}' | tr -d \", )
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): Failed to create snapshot of volume $volid."
           exit $rc
        fi
        echo " SnapId : " $snapid >> ebs_snap.info
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): Failed to update ebs_snap.info with SnapId : $snapid."
           exit $rc
        fi
        echo " MountPt : " $mountpt >> ebs_snap.info
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): Failed to update ebs_snap.info with MountPt : $mountpt."
           exit $rc
        fi
        echo " OrigVolumeId : " $volid >> ebs_snap.info
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): Failed to update ebs_snap.info with OrigVolumeId : $volid."
           exit $rc
        fi
        echo "$MY_NAME $(date): Snapshot was successful for volume $volid snapshot $snapid."
        exit 0

        ;;
    $PRERESTORE )
        if [ ! -f ebs_snap.info ]; then
           echo "$MY_NAME $(date): snasphot info file is not present, exiting normally."
           exit 0
        fi
        usev2="false"
        nl=$(nvme list)
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to run nvme command."
           exit $rc
        fi
        if [ -z $nl ]; then
           usev2="true"
        fi
        snapid=$(set -o pipefail; /bin/cat ebs_snap.info | awk '/SnapId :/{print $3}')
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to read SnapId."
           exit $rc
        fi
        origvolid=$(set -o pipefail; /bin/cat ebs_snap.info | awk '/OrigVolumeId :/{print $3}')
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to read original volume id."
           exit $rc
        fi
        volumetype=$(set -o pipefail; /bin/cat ebs_snap.info | awk '/VolumeType :/{print $3}')
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to read original volume type."
           exit $rc
        fi
        iops=$(set -o pipefail; /bin/cat ebs_snap.info | awk '/Iops :/{print $3}')
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to read original volume iops."
           exit $rc
        fi
        mountpt=$(set -o pipefail; /bin/cat ebs_snap.info | awk '/MountPt :/{print $3}')
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to read MountPt."
           exit $rc
        fi
        if [ -d $mountpt ]; then
          mpts=$(set -o pipefail; df | awk '{print $6}')
          rc=$?
          if [ $rc != 0 ]; then
             echo "$MY_NAME $(date): unable to run df."
             exit $rc
          fi
          for m in $mpts ;do
             if [ "$m" == "$mountpt" ]; then
                echo "$MY_NAME $(date): mount point $mountpt busy. "
                exit 1
             fi
          done
        else
           mkdir -p $mountpt
           rc=$?
           if [ $rc != 0 ]; then
              echo "$MY_NAME $(date): unable to mkdir $mountpt."
              exit $rc
           fi
        fi
        echo "$MY_NAME $(date): prepared mountpt $mountpt."
        xinstid=$(set -o pipefail; /bin/cat ebs_snap.info | awk '/instanceId :/{print $3}')
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to read checkpoint instanceId."
           exit $rc
        fi
        instid=$(/bin/cat /var/lib/cloud/data/instance-id || exit 1)
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to read current instanceId."
           exit $rc
        fi
        azone=$(set -o pipefail; /bin/cat ebs_snap.info | awk '/availabilityZone :/{print $3}')
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to read availabilityZone."
           exit $rc
        fi
        echo "$MY_NAME $(date): creating clone of $snapid in $azone."
        if [ $volumetype == "gp3" ] || [ $volumetype == "io1" ] || [ $volumetype  == "io2" ]; then
           volid=$(set -o pipefail; aws ec2 create-volume --volume-type $volumetype --snapshot-id $snapid --availability-zone $azone --iops $iops \
                |  awk '/VolumeId/{print $2}' | tr -d \", )
        else
           volid=$(set -o pipefail; aws ec2 create-volume --volume-type $volumetype --snapshot-id $snapid --availability-zone $azone \
                |  awk '/VolumeId/{print $2}' | tr -d \", )
        fi
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to create volume from snapshot $snapid with availability zone $azone."
           exit $rc
        fi
        retry=0
        while [ $retry -lt 10 ]; do
           status=$(set -o pipefail; aws ec2 describe-volumes --volume-id  $volid \
                  | awk '/State/{print $2}' | tr -d \", )
           if [ "$status" == "available" ]; then
              break
           fi
           echo "$MY_NAME $(date): waiting for $volid to become available."
           sleep 5
           ((retry++))
        done
        if [ "$status" != "available" ]; then
           echo "$MY_NAME $(date): volume $volid not getting ready $status."
           aws ec2 delete-volume --volume-id $volid
           if [ $? != 0 ]; then
              echo "$MY_NAME: During cleanup, unable to delete $volid."
           fi
           return 1
        fi

        #find a non used /dev/sd device. https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/device_naming.html
        device=$(findunused)
        if [ $? != 0 ]; then
           echo "$MY_NAME $(date): no device available to mount."
           aws ec2 delete-volume --volume-id $volid
           if [ $? != 0 ]; then
              echo "$MY_NAME: During cleanup, unable to delete $volid."
           fi
           return 1
        fi
        if [ $usev2 == "true" ]; then
           savelist=$(set -o pipefail; lsblk --output NAME | grep ^xv)
        else
           savelist=$(set -o pipefail; nvme list 2>/dev/null | awk '/^\/dev\//{print $1}')
        fi
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to get device list."
           aws ec2 delete-volume --volume-id $volid
           if [ $? != 0 ]; then
              echo "$MY_NAME $(date): During cleanup, unable to delete $volid."
           fi
           return $rc
        fi
        echo "$MY_NAME $(date): attaching $volid to $instid at $device."
        aws ec2 attach-volume --volume-id $volid --instance-id $instid --device $device
        rc=$?
        if [ $rc != 0 ]; then
           echo "$MY_NAME $(date): unable to attach volume $volid on instance $instid at device $device."
           aws ec2 delete-volume --volume-id $volid
           if [ $? != 0 ]; then
              echo "$MY_NAME $(date): During cleanup, unable to delete $volid."
           fi
           exit $rc
        fi

        retry=0
        while [ $retry -lt 10 ]; do
           status=$(set -o pipefail; aws ec2 describe-volumes --volume-id  $volid \
                  | head -n 12 | awk '/State/{print $2}' | tr -d \", )
           if [ "$status" == "attached" ]; then
              break
           fi
           echo "$MY_NAME $(date): waiting for $volid to get attached to $instid."
           sleep 5
           ((retry++))
        done
        if [ "$status" != "attached" ]; then
           echo "$MY_NAME $(date): volume $volid not getting ready $status."
           aws ec2 delete-volume --volume-id $volid
           if [ $? != 0 ]; then
              echo "$MY_NAME $(date): During cleanup, unable to delete $volid."
           fi
           return 1
        fi

        if [ $usev2 == "true" ]; then
           newdev=$(findnewdev_v2 "${savelist[@]}")
           rc=$?
        else
           newdev=$(findnewdev "${savelist[@]}")
           rc=$?
        fi
        if [ $rc == 0 ]; then
           echo "$MY_NAME $(date): mounting $newdev at $mountpt."
           mount $newdev $mountpt
           rc=$?
           if [ $rc != 0 ]; then
              echo "$MY_NAME $(date): mount device $newdev on $mountpt failed."
           fi
        else
           echo "$MY_NAME $(date): unable to find the new block device."
        fi
        if [ $rc != 0 ]; then
           aws ec2 detach-volume --volume-id $volid --force
           if [ $? != 0 ]; then
                echo "$MY_NAME $(date): During cleanup, unable to detach volume $volid."
           else
              # let the dust settle after detach.
              sleep 5
              aws ec2 delete-volume --volume-id $volid
              if [ $? != 0 ]; then
                 echo "$MY_NAME $(date): cleanup unable to delete $volid."
              fi
           fi
           exit $rc
        fi
        if [ "$xinstid" == "$instid" ]; then
            # we are restoring on same instance, best effort to detach the original volume
            aws ec2 detach-volume --volume-id $origvolid
            if [ $? != 0 ]; then
                echo "$MY_NAME $(date): unable to detach original volume $origvolid but not treating it as failure."
            fi
        fi
        echo "$MY_NAME $(date): Restore successful new volume $volid on $mountpt."
        exit 0
        ;;
esac

exit 0
