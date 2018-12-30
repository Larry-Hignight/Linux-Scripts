#!/bin/tcsh

if($1 == '') then
	echo 'need to include the file name with path'
else
        wget -r "ftp://server2.shellworld.net${1}" --ftp-user=chime --ask-password
endif

