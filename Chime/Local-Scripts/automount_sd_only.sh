#!/bin/bash

OUTPUT=`sudo -u chime grep "mountable" /var/log/syslog | tail -1 |  awk '{print $NF}'`

if mount | grep /media/$OUTPUT > /dev/null; then
   echo "unmounting /media/$OUTPUT"
   pumount --yes-I-really-want-lazy-unmount /dev/$OUTPUT
else
   echo "mounting /media/$OUTPUT"
   pmount /dev/$OUTPUT $OUT$PUT
fi


