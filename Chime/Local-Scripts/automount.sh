#!/bin/bash

OUTPUT=`sudo -u chime grep "mountable" /var/log/syslog | tail -1 |  awk '{print $NF}'`

if mount | grep /media/"${OUTPUT//[[:digit:]]/}" > /dev/null; then
   echo "unmounting"
#   echo "/dev/${OUTPUT//[[:digit:]]/}"
   pumount --yes-I-really-want-lazy-unmount "/dev/${OUTPUT//[[:digit:]]/}"
   # rmdir /auto/"${OUTPUT//[[:digit:]]/}"
else
   echo "mounting"
   #mkdir -p /auto/"${OUTPUT//[[:digit:]]/}"
   chown -R chime:chime /auto/*
#   echo "/dev/${OUTPUT//[[:digit:]]/}"
#   echo "/auto/${OUTPUT//[[:digit:]]/}"
#   mount -o umask=000 "/dev/${OUTPUT//'[[:digit:]]/}" "/auto/${OUTPUT//[[:digit:]]/}"
   pmount "/dev/${OUTPUT//[[:digit:]]/}" "${OUTPUT//[[:digit:]]/}"
fi


