#!/bin/sh

echo "uploading image"
adb push $1 /sdcard/
name=`basename $1`
echo "flashing modem image"
adb shell "su -c dd if=/sdcard/$name  of=/dev/block/platform/msm_sdcc.1/by-name/modem"
