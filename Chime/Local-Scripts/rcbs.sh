#!/bin/tcsh

if($1 == '') then
	echo 'need to include the file name with path'
else
	cvlc https://www.cbsnews.com/common/video/cbsn_header_prod.m3u8 --sout "#transcode{vcodec=none,acod=mp3,ab=70,channels=2,samplerate=44100}:std{access=file{no-overwrite},mux=mp3,dst='${1}'}"
endif

