#! /bin/bash
if [ $# -eq 1 ]
  then
#    echo "one"
    mplayer -cdrom-device /dev/sr0 cdda:// -cache 5000 -ao alsa:device=hw="${1}"

  else
#    echo "two ${1}"
    mplayer -cdrom-device /dev/sr0 cdda://"${2}" -cache 5000 -ao alsa:device=hw="${1}"

fi
