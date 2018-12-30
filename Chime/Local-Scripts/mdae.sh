#! /bin/bash
if test "$#" -ne 1; then
    echo "Must include path and name of mp3 file"
    exit 0
elif [ ! -f $1 ]; then
   echo "File $FILE does not exists."
   exit 0
elif [[ ! ${1: -4} == ".mp3" ]] && [[ ! ${1: -4} == ".aac" ]]  ; then
	echo "You must use a mp3 or aac file"
	exit 0
fi

CURRENT=`pwd`

echo "Converting to wav for DAE"

ffmpeg -y -loglevel panic -i $1 $CURRENT/dae_temp.wav

echo "Opening in DAE"

dae $CURRENT/dae_temp.wav

echo "Converting back to mp3"

ffmpeg -loglevel panic -i $CURRENT/dae_temp.wav $CURRENT/`basename $1`

rm $CURRENT/dae_temp.wav
