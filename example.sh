#!/bin/sh

DEVICE="/dev/watchdog"
TIMEOUT=30

watchpuppy -d $DEVICE -t $TIMEOUT start

while true
do
  if [ everything_is_working_correctly ] ; then
    sleep 20
  else
    reboot # not needed but will speed up reboot
    exit 1
  fi
done
