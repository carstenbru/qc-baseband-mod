#!/bin/bash

SPLITSIZE=$(expr 500000000 - 10000) #TODO adapt
POLL_INTERVAL=20 #TODO adapt

usage() {
    echo "Usage:" $0 "LOCAL_TARGET_DIR ANDROID_DUMP_BASE_DIR:"
    echo "    LOCAL_TARGET_DIR: local directory where the dumps will be stored"
    echo "    ANDROID_DUMP_BASE_DIR: base dir name of the dump on the Android phone. As entered in the App without filename (full path required)."
}

pull_files() {
    echo "checking for new segments on device"
    adb start-server
    adb shell ls -l "$2*" > android.files
    
    while IFS=  read -r file; do
        echo file $file
        farr=($file)
        #echo farr ${farr[6]}
        #if [ ${#farr[@]} -ge 6 ]; then
            if [ $SPLITSIZE -le ${farr[3]} ]; then
                remote=$2/${farr[6]::-1}
                echo "pulling file:" $remote
                adb pull $remote $1
                adb shell rm $remote
            fi
        #fi
    done < android.files

}

if [ $# -ne 2 ]; then
    usage
    exit
fi

while [ 1 -eq 1 ]; do
    pull_files $1 $2
    sleep $POLL_INTERVAL
done
