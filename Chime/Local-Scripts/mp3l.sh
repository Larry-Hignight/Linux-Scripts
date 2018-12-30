#!/bin/bash
seconds=`/usr/bin/mp3info -p %S ${1}`

convertsecs() {
 ((h=${1}/3600))
 ((m=(${1}%3600)/60))
 ((s=${1}%60))
 printf "%2d hours and %2d minutes and %2d seconds\n" $h $m $s
}

echo $(convertsecs $seconds)

