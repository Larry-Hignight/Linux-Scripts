#!/bin/tcsh

if($1 == '' && $2 =='') echo 'need to include first the stream and then the file name with path'

echo $1
echo $2


vlc -vvv $1 --sout=file/ps:$2

