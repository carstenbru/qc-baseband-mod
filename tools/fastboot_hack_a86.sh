#!/bin/sh

# write broken recovery
adb shell "su -c dd if=/dev/zero of=/dev/block/platform/msm_sdcc.1/by-name/recovery bs=2048 count=6995; sync;"
# reboot to recovery
adb reboot recovery
