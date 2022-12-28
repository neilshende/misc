#!/bin/bash
while (( 1 )); do
    att=$(networksetup -getairportnetwork en0 | grep "Current Wi-Fi Network:" | awk '{print $4}')
    if [[ "$att" != ATT5zuh9zh ]]; then
        echo $(date) Problem detected. Switching to hotspot
        /System/Library/PrivateFrameworks//Apple80211.framework/Versions/A/Resources/airport -setsirportnetwork en0 ATT5zuh9zh 6k2q58ysy%b5
        networksetup -setairportpower en0 on

    fi
    sleep 1
done


#!/bin/bash
list=$(/System/Library/PrivateFrameworks//Apple80211.framework/Versions/A/Resources/airport -s | awk '{print $1}')

While (( 1 )); do
while (( 1 )); do
    wifi=$(networksetup -getairportnetwork en0 | grep "Current Wi-Fi Network:" | awk '{print $4}')
    if [[ "$wifi" != "$1" ]]; then
        echo $(date) Problem detected. Switching to $3
        /System/Library/PrivateFrameworks//Apple80211.framework/Versions/A/Resources/airport -setsirportnetwork en0 $3 $4
        networksetup -setairportpower en0 on
        break
    fi
    sleep 1
done
while (( 1 )); do
    wifi2=$(networksetup -getairportnetwork en0 | grep "Current Wi-Fi Network:" | awk '{print $4}')
    if [[ "$wifi2" != "$3" ]]; then
        echo $(date) Problem detected. Switching to $1
        /System/Library/PrivateFrameworks//Apple80211.framework/Versions/A/Resources/airport -setsirportnetwork en0 $1 $2
        networksetup -setairportpower en0 on
        break
    fi
    sleep 1
done
done
