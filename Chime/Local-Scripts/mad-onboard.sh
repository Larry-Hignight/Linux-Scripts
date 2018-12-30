#!/bin/tcsh

if($1 == '') echo 'need to include the file name and path'


#madplay -o wave:- $1 | aplay -D onboard
madplay -o wave:- $1 | aplay -Dfront:CARD=Intel,DEV=0

