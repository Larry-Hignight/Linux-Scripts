#!/bin/sh
#############################################################################
#
#   autolame  $Revision: 1.17 $  $Date: 2003/04/04 19:39:21 $
#   automatic encoding of .wav files
#
#   Copyright (C) 2000  Christian Garbs <mitch@uni.de>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#############################################################################
#
#   Get the most current version here:
#   http://www.h.shuttle.de/mitch/stuff.en.html
#
#   $Id: autolame.sh,v 1.17 2003/04/04 19:39:21 mitch Exp $
#
#############################################################################
#
# INSTALLATION:
#
# Copy this script into a directory (this directory will be the autolame
# root directory). Then create two subdirectories called 'in' and 'out'
# and edit the following variables to your needs:
#
#
# ===> autolame root
#
ROOTDIR=/usr/local/autolame
#
#
# ===> lockfile
#      (it's best to remove this file on every bootup)
#
LOCKFILE=$ROOTDIR/autolame.LOCK
#
#
# ===> encoder (this doesn't need to be lame, others will work, too)
#
ENCODER=/usr/local/bin/lame
ENCODERPARAMS=-hk
#
# another encoder:
#ENCODER=/usr/local/bin/notlame
#ENCODERPARAMS="-h -p -t -b 160"
#
#
# ===> where to show status messages
#      
# current console/no redirection:
SCREEN=
#
# on this tty (must be writable for this user!)
#SCREEN=/dev/tty11
#
# be quiet:
#SCREEN=/dev/null
#
# logfile:
#SCREEN=/var/log/autolame.log
#
#
#
# ===> nice level
#
NICE=13
#
#
# ===> rescan file (this is just used as a flag)
#
RESCAN=$ROOTDIR/autolame.RESCAN
#
#
# ===> external programs:
#
# If your $PATH is set correctly, you don't need explicit paths:
BASENAME=basename
DATE=date
DIRNAME=dirname
FIND=find
MKDIR=mkdir
MV=mv
RENICE=renice
RM=rm
RMDIR=rmdir
SED=sed
TOUCH=touch
#
# This works fine for my SuSE 6.3:
#BASENAME=/usr/bin/basename
#DATE=/bin/date
#DIRNAME=/usr/bin/dirname
#FIND=/usr/bin/find
#MKDIR=/bin/mkdir
#MV=/bin/mv
#RENICE=/usr/bin/renice
#RM=/bin/rm
#RMDIR=/bin/rmdir
#SED=/usr/bin/sed
#TOUCH=/usr/bin/touch
#
# These should do on Red Hat 6.2:
#BASENAME=/bin/basename
#DATE=/bin/date
#DIRNAME=/bin/dirname
#FIND=/usr/bin/find
#MKDIR=/bin/mkdir
#MV=/bin/mv
#RENICE=/usr/bin/renice
#RM=/bin/rm
#RMDIR=/bin/rmdir
#SED=/bin/sed
#TOUCH=/bin/touch
#
#
#
# This script should be called periodically (e.g. from cron).
# Simply put your .wav files into the 'in' directory and wait a bit.
# The encoded files will written to the 'out' directory (LOOK OUT: duplicate
# names will be overwritten silently!).  The already encoded .wav files will
# be erased.
#
# If you call this script with "-s" (silent) as an argument, the
# message "already running!" will not be displayed if another instance
# if autolame is running.
#
#############################################################################
#
# YOU DON'T NEED TO EDIT ANYTHING BEYOND...
#
#############################################################################
#
# History:
#
# $Log: autolame.sh,v $
# Revision 1.17  2003/04/04 19:39:21  mitch
# - Sane default value for $SCREEN (use current console).
# - Check for $ENCODER and $ROOTDIR on startup.
#
# Revision 1.16  2000/10/30 20:21:45  mitch
# - Fix for a bug that would eventually delete the "/in" directory when
#   it was empty.  Fixed by: Gerrit van den Hanenberg <gvdnh@xs4all.nl>
#
# Revision 1.15  2000/10/27 17:50:43  mitch
# - .wav suffix of input files is not case sensitive any more.
# - Empty '/in' subdirectories are removed.
#
# ( both from a patch by Gerrit van den Hanenberg <gvdnh@xs4all.nl> )
#
# Revision 1.14  2000/10/19 19:39:50  mitch
# - All external command locations should be configurable now.
#   This is based partly on a patch by Jon Nelson <john@debian.org>
#
# Revision 1.13  2000/09/21 18:02:36  mitch
# - Revision number is shown in startup.
#
# Revision 1.12  2000/09/21 17:57:27  mitch
# - More external command locations can be configured.
# - ANSI color sequences are stored in variables (easier to read).
#
# Revision 1.11  2000/09/21 17:43:14  mitch
# - Locations of external commands can be configured.
#
# Revision 1.10  2000/09/20 07:51:44  mitch
# - BUGFIX: a broken symbolic link in the in directory would cause an
#           infinite loop while checking for new files
#
# Revision 1.9  2000/09/08 20:59:27  mitch
# - check for new files at end of list has been deactivated due to
#   problems with symbolic links (a broken link will create an infinite
#   loop with a 'file not found' error every time)
#
# Revision 1.8  2000/09/02 16:52:31  mitch
# - when all files have been encoded, a check for new files is performed
#
# Revision 1.7  2000/09/02 16:42:46  mitch
# - the .wav file is encoded to a .tmp file, which is renamed to .mp3
#   on completion
#
# Revision 1.6  2000/08/22 19:04:33  mitch
# - fixed a typo
#
# Revision 1.5  2000/08/22 19:03:01  mitch
# - "-s" supresses "already running!" message
#
# Revision 1.4  2000/08/22 18:46:06  mitch
# - directories now may contain spaces
#
# Revision 1.3  2000/08/22 18:18:53  mitch
# - support of subdirectories in the 'in/' directory
# - fixed a bug with filenames that contain ".wav" somewhere in the middle
# - more colorful output :-)
#
# Revision 1.2  2000/08/22 17:40:45  mitch
# - GPL, header and documentation added
# - $SCREEN can be set to "" meaning no output redirection
#
#############################################################################

# Ansi color sequences
_CYAN="\033[36m"
_GREEN="\033[32m"
_NORMAL="\033[m"
_RED="\033[31m"
_WHITE="\033[1m"

# redirect stdout
if test -n "$SCREEN"; then
    exec >> $SCREEN
fi

# be nice
$RENICE $NICE $$ > /dev/null

# check for rootdir
if ! test -d $ROOTDIR; then
    echo "can't find root directory: ROOTDIR=$ROOTDIR" 1>&2
    exit 1
fi

# check for encoder
if ! test -e $ENCODER; then
    echo "encoder binary not found: ENCODER=$ENCODER" 1>&2
    exit 1
fi
if ! test -x $ENCODER; then
    echo "encoder binary can't be executed: ENCODER=$ENCODER" 1>&2
    exit 1
fi

# check for lockfile
if test -e $LOCKFILE; then
    if test "$1" != "-s"; then
	echo -e "autolame [`$DATE +%D\ %T`] : ${_RED}already running!${_NORMAL}"
    fi
    exit 0
fi
echo $$ > $LOCKFILE || echo -e "autolame [`$DATE +%D\ %T`] : ${_RED}ERROR during lockfile creation${_NORMAL}"

# start message
echo
echo -e "autolame [`$DATE +%D\ %T`] : "`$BASENAME $0`' $Revision: 1.17 $'

# loop
cd $ROOTDIR
$TOUCH "$RESCAN"
while test -e "$RESCAN"; do

    echo -e "autolame [`$DATE +%D\ %T`] : checking for files"
    $RM -f "$RESCAN"

    $FIND in -name \*.[wW][aA][vV] | \
    while read FILE_ORIG; do

	# mangle the file name
	FILE=`echo "${FILE_ORIG}"|$SED 's/\.wav$//i'|$SED 's/^in\///'`
	DIR=`$DIRNAME "$FILE"`
	SONG=`$BASENAME "$FILE"`
    
	# create output directory
	$MKDIR -p "out/$DIR"

	# encode a file and eventually delete it
	echo -e "autolame [`$DATE +%D\ %T`] : <${_CYAN}$DIR${_NORMAL}/${_WHITE}$SONG${_NORMAL}> ${_WHITE}\033[33mstart${_NORMAL}"
	$ENCODER $ENCODERPARAMS "${FILE_ORIG}" "out/$DIR/$SONG.tmp" 2> /dev/null \
	    && ($MV "out/$DIR/$SONG.tmp" "out/$DIR/$SONG.mp3" \
		&& ($RM "${FILE_ORIG}" \
		    && ( echo -e "autolame [`$DATE +%D\ %T`] : <${_CYAN}$DIR${_NORMAL}/${_WHITE}$SONG${_NORMAL}> ${_GREEN}finished${_NORMAL}" ; exit 0 ) \
		    || (echo -e "autolame [`$DATE +%D\ %T`] : <${_CYAN}$DIR${_NORMAL}/${_WHITE}$SONG${_NORMAL}>  ${_RED}ERROR during removing${_NORMAL}" ; exit 1 )) \
		|| ( echo -e "autolame [`$DATE +%D\ %T`] : <${_WHITE}$SONG${_NORMAL}>  ${_RED}ERROR during rename${_NORMAL}" ; exit 1)) \
	    || ((echo -e "autolame [`$DATE +%D\ %T`] : <${_WHITE}$SONG${_NORMAL}>  ${_RED}ERROR during encoding${_NORMAL}" \
		&& $RM "out/$DIR/$SONG.tmp" ; exit 1 \
		|| echo -e "autolame [`$DATE +%D\ %T`] : <${_WHITE}$SONG${_NORMAL}>  ${_RED}ERROR during removal of stale tmp file${_NORMAL}") ; exit 1 ) \
		&& $TOUCH "$RESCAN"

        # remove empty subdirectory from in directory

        if test "$DIR" != "."; then
	    cd in
	    $RMDIR -p "$DIR" 2> /dev/null
	    cd ..
	fi

    done

done

# end message
echo -e "autolame [`$DATE +%D\ %T`] : no more files"

# remove rescan file
$RM -f "$RESCAN"

# remove lockfile
$RM -f "$LOCKFILE"
