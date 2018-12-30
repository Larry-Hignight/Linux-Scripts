#! /bin/bash
if test "$#" -ne 1; then
    echo "Must include path and name of mp3 file"
    exit 0
elif [ ! -f $1 ]; then
   echo "File $FILE does not exists."
   exit 0
elif [[ ! ${1: -4} == ".mp3" ]]; then
	echo "You must use a mp3 file"
	exit 0
fi

echo "Checking for silence"

ffmpeg -i $1 -af silenceremove=1:0:-50dB -loglevel panic output.mp3
mv output.mp3 $1 && rm output.mp3

echo "done and stuff"
