#!/bin/bash
for i in *.mp3; do
  echo "checking "$i
  cp $i out.mp3
  rm $i
  ffmpeg -loglevel panic -i out.mp3 -c:a copy -vn $i
  rm out.mp3
done
