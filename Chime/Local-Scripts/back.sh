#!/bin/bash

#    Copyright 2010-2014 Eric Dujardin.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

# VERSION 1.3

# HISTORY
#  v1.0: initial release
#  v1.1: cope with platforms on which oggenc has no built-in flac support, added HISTORY section
#  v1.2 : source WMA support (preliminary), source & target ALAC support, bug fixes, source & target WAV support
#  v1.2.1 : minor bug fixes
#  v1.2.2 : Oops, WMA source support wasn't just there.
#  v1.2.3 : More AAC/ALAC extensions, completed dependency list, improved output, tunable encoding options
#  v1.3	  : Use multiple processes for faster compression using multicores, optimize flac->ogg conversion, remove leftover message
# TODO: support OPUS

# DESCRIPTION
# BatchAudioConvert aka BAC version 1.3
# BAC is a command-line tool that converts, to a target audio file format, 
#   all the audio files stored in a directory tree of any depth.
# Supported file formats are OGG, FLAC, MP3, AAC, ALAC, WAV, in all ways. The mp3 
#   target supports both CBR and VBR modes. WMA is supported as source format.
# Main audio tags are preserved in the process, more precisely the title, 
#   artist, album, year, genre and track number.
# The program attempts carefully at supporting most characters in file names, 
#   especially white spaces.
# This program runs on Linux or Cygwin. Adaptation to other UNIXes should 
#   be straightforward, as well as any OS on which a Bourne-like shell is 
#   available.
# This program is not reentrant, i.e; it can't be run several times at once against the same 
#    input files.
# This program is a shell script designed to be easily adapted to other formats, 
#   OSes or preferences.

# DEPENDENCIES
# This program requires the following software to be installed:
# flac and metaflac (flac files only)
# vorbis-tools (ogg files only)
# faac and faad (aac files only)
# lame and madplay (mp3 files only)
# alac-decoder and mpeg4ip-utils (ALAC decode)
# ffmpeg (ALAC encode, wma decode)
# mplayer (wma decode)
# mp4v2-utils (ALAC, AAC, WAV decode)

# MULTIPROCESSING
# This version is the first to support multiprocessing. More precisely, all conversions are now done in subprocesses, and
# a variable MAX_PROC defines the maximum number of simultaneous processes that can be used for conversions. This allows
# both to tune the amount of system resources to be used for conversions, and to avoid performance trashing due to too 
# many processes. When MAX_PROC is set to 1, a single subprocess is used, and BAC operates as efficiently as previously.  
# For optimal performances, it should be set to the number of available CPU threads.
# Early tests show impressive improvements, feel free to report your joy or disappointment !

# User encoding options
OGGENC_OPTS="" # use OGGENC default
CBRENC_OPTS="--preset cbr 128" # for lame 
VBRENC_OPTS="--preset standard" # for lame 
FLACENC_OPTS=""  # use FLACENC default
AACENC_OPTS=""   # use FAAC default
ALACENC_OPTS=""  # ffmpeg encoding options for ALAC: use ffmpeg defaults

MAX_PROC=4       # set maximum number of encoding sub-processes
# set this on Linux for optimal performance: number of available CPU threads
#MAX_PROC=`cat /proc/cpuinfo | grep processor | wc -l`

help() {
	echo ============ Batch Audio Convert =============
	echo Mass conversion of any directory tree of audio files. 
	echo; echo Usage:
	echo "       " bac.sh '[ogg|cbr|vbr|flac|aac|alac|wav [src_dir [dst_dir]] ]' 
	echo; echo The first argument is the target format, where cbr and vbr stand resp. for constant- '(128k)' and variable- '(standard)' bit rate MP3. By default, '"ogg"' is taken.
	echo; echo The second argument is the source directory. By default, '"./src"' is taken.
	echo; echo The third argument is the target directory. By default, '"./dst"' is taken.
	echo; echo The program relies on the file extensions to guess their type, no content inspection is performed to guess. The only exception is m4a. Extensions are taken as follows:
	echo "    Extension         Type"
	echo "    .flac             FLAC"
	echo "    .mp3              MP3 (cbr or vbr)"
	echo "    .m4a,.mp4,.aac    AAC or ALAC"
	echo "    .ogg              OGG"
	echo "    .wav              WAV"
	echo "    .wma              WMA"
	echo; echo Just change the values of *_OPTS variables in bac.sh to tune the encoding options. 
	echo Change the value of MAX_PROC in bac.sh to the number of CPU threads for optimal performance.
}

dupdir() {
    local d
    for d in *
    do
	if [ -d "$d" ] 
	then
	    mkdir -p "$dstdir"/"$1"/"$d"
	    cd "$d"
	    dupdir "$1"/"$d"
	    cd ..
	fi
    done
}

generictag() {
	local f="$1"
	local tags=`mktemp`
	local tagname
# extract IDs providing the number of tags, and their names and values as list of local variables 
	mplayer -identify "$f" -endpos 0 2> /dev/null | grep '^ID_CLIP_INFO_' | sed 's/\([^=]*\)=\([^"].*\)/local \1="\2"/' > $tags
	source $tags
# now go through this list to built a set of variables tag_NAME=VALUE
	i=0
	while [ $i -ne ${ID_CLIP_INFO_N:-0} ]
	do
		eval tagname='$'ID_CLIP_INFO_NAME$i
# as / is not good in a var name, substitute WM/name with WMname
		if expr "$tagname" : 'WM/.*' > /dev/null
		then
			tagname=`echo $tagname | sed 's:WM/:WM:'`
		fi
		eval tag_$tagname='"$'ID_CLIP_INFO_VALUE$i'"'
		i=`expr $i + 1`
	done
	rm $tags
}

flactag() {
    local f="$1"
    songtitle=`metaflac --show-tag=TITLE "$f"| cut -d= -f2` 
    artist=`metaflac --show-tag=ARTIST "$f"| cut -d= -f2`
    album=`metaflac --show-tag=ALBUM "$f"| cut -d= -f2`
    year=`metaflac --show-tag=DATE "$f"| cut -d= -f2`
    track=`metaflac --show-tag=TRACKNUMBER "$f"| cut -d= -f2`
    genre=`metaflac --show-tag=GENRE "$f"| cut -d= -f2`
}
    
mp3tag() {
    local f="$1"
    local tags=`mktemp`
    madplay -T "$f" 2>&1 | grep ": " | sed -e 's/ *\(.*\): \(.*\)/\1=\2/' > $tags
    songtitle=`grep Title $tags | cut -d= -f2`
    artist=`grep Artist $tags | cut -d= -f2`
    album=`grep album $tags | cut -d= -f2`
    year=`grep Year $tags | cut -d= -f2`
    track=`grep track $tags | cut -d= -f2`
    genre=`grep genre $tags | cut -d= -f2`
    rm $tags
}


oggtag() {
    local f="$1"
    local tags=`mktemp`
    vorbiscomment -l "$f" > $tags
    songtitle=`grep TITLE $tags | cut -d= -f2`
    artist=`grep ARTIST $tags | cut -d= -f2`
    album=`grep ALBUM $tags | cut -d= -f2`
    year=`grep YEAR $tags | cut -d= -f2`
    track=`grep TRACKNUMBER $tags | cut -d= -f2`
    genre=`grep GENRE $tags | cut -d= -f2`
    rm $tags
}

wmatag() {
# tags and mapping from http://msdn.microsoft.com/en-au/library/dd743066%28VS.85%29.aspx and http://help.mp3tag.de/main_tags.html
	generictag "$1"
	songtitle="${tag_name:-}" 
	artist="${tag_author:-}"
	album="${tag_WMAlbumTitle:-}"
	year="${tag_WMYear:-}"
	track="${tag_WMTrackNumber:-}"
	genre="${tag_WMGenre:-}"  
#	echo $songtitle $artist $album $year $track $genre
	unset tag_name tag_author tag_WMAlbumTitle tag_WMYear tag_WMTrackNumber tag_WMGenre
}

aactag() {
    local f="$1"
    local tags=`mktemp`
    faad -i "$f" 2>&1 | grep ": " | sed -e 's/: /=/' > $tags
    songtitle=`grep title $tags | cut -d= -f2`
    artist=`grep artist $tags | cut -d= -f2`
    album=`grep album $tags | cut -d= -f2`
    year=`grep date $tags | cut -d= -f2`
    track=`grep track $tags | cut -d= -f2`
    genre=`grep genre $tags | cut -d= -f2`
    rm $tags
}

alactag() {
    local f="$1"
    local tags=`mktemp`
    mp4info "$f" > $tags 
    songtitle=`grep Name $tags | sed 's/ Name: \(.*\)/\1/'`
    artist=`grep Artist $tags | sed 's/ Artist: \(.*\)/\1/'`
    album=`grep Album $tags | sed 's/ Album: \(.*\)/\1/'`
    year=`grep Year $tags | sed 's/ Year: \(.*\)/\1/'`
    track=`grep Track $tags | sed 's/ Track: \(.*\)/\1/'`
    genre=`grep Genre $tags | sed 's/ Genre: \(.*\)/\1/'`   
    rm $tags
}

mp4tag() {
    local f="$1"
    mp4type=`mp4info "$f" | sed '1,3d
4s/^[0-9]*[ \t]*audio[ \t]*\([^,]*\),.*/\1/
4q'`
    if [ "$mp4type" == alac ]
    then
	alactag "$f"
    else
        aactag "$f"
    fi
}	

wavtag() {
    local f="$1"
    local tags=`mktemp`
    mp4info "$f" > $tags 
    songtitle=`grep Name $tags | sed 's/ Name: \(.*\)/\1/'`
    artist=`grep Artist $tags | sed 's/ Artist: \(.*\)/\1/'`
    album=`grep Album $tags | sed 's/ Album: \(.*\)/\1/'`
    year=`grep Year $tags | sed 's/ Year: \(.*\)/\1/'`
    track=`grep Track $tags | sed 's/ Track: \(.*\)/\1/'`
    genre=`grep Genre $tags | sed 's/ Genre: \(.*\)/\1/'`   
    rm $tags
}

unogg() {
	oggdec -Q "$1" -o tempo${BASHPID}.wav
}

unflac() {
	flac -s -d -f -o tempo${BASHPID}.wav "$1"
}

unmp3() {
	madplay "$1" -Q -o tempo${BASHPID}.wav 
}

unwma() {
	ffmpeg -i "$1" tempo${BASHPID}.wav 2> /dev/null
}

unaac() {
	faad -q -o tempo${BASHPID}.wav -f 1 "$1"
}

unalac() {
	alac-decoder -f tempo${BASHPID}.wav "$1"
}

unmp4() {
	if [ "$mp4type" == alac ]
	then
		unalac "$1"
	else
		unaac "$1"
	fi
}

unwav() {
	cp "$1" tempo${BASHPID}.wav
}

toogg() {
	oggenc --quiet -a "$artist" -t "$songtitle" -l "$album" -d "$year" -N "$track" -G "$genre" $OGGENC_OPTS -o "$dstdir"/"$2" "$1" 
}

_try_flac2ogg=1
autodecode_flac2ogg() {
	if [ $_try_flac2ogg  -eq 1 ]
	then if toogg "$1" "$2" 2>/dev/null 
	     then true
	     else _try_flac2ogg=2; false
	     fi
	else
	     false
	fi

}

toflac() {
	flac -s "$1" -T TITLE="$songtitle" -T ARTIST="$artist" -T ALBUM="$album" -T DATE="$year" -T TRACKNUMBER="$track" -T GENRE="$genre" $FLACENC_OPTS -o "$dstdir"/"$2"
}

tomp3() {
	lame --quiet $3 --tt "$songtitle" --ta "$artist" --tl "$album" --ty "$year" --tn "$track" --tg "$genre" "$1" "$dstdir"/"$2"
}

towav() {
	cp "$1" "$dstdir"/"$2"	
}

toaac() {
	faac --title "$songtitle" --artist "$artist" --album "$album" --year "$year" --track "$track" --genre "$genre" $AACENC_OPTS -o "$dstdir"/"$2"  "$1" 2> /dev/null
}

toalac() {
# http://multimedia.cx/eggs/supplying-ffmpeg-with-metadata/
# 21/10/2009 writing metadata to FLAC is not supported yet.
	ffmpeg -i "$1" -acodec alac -metadata author="$artist" -metadata title="$songtitle" -metadata year="$year" -metadata album="$album" -metadata track="$track" -metadata genre="$genre" $ALACENC_OPTS "$dstdir"/"$2"
}

cvtmsg() {
	echo converting "$1" to "$2"
}

copymsg() {
	echo copying "$1"
}

cvtdir() {
    local d f
    for d in *
    do
	if [ -d "$d" ] 
	then
	    cd "$d"
	    cvtdir "$1"/"$d"
	    cd ..
	fi
    done
    for f in *.ogg
    do
      if [ "$f" != '*.ogg' ]
      then
	  child_start  
	  ( oggtag "$f"
	  if [ $tofmt == ogg ]
	  then
		copymsg "$f" 
	  	cp "$f" "$dstdir"/"$1"/"$f"
      	  fi
 	  if [ $tofmt == aac ]
      	  then
		g=`basename "$f" .ogg`.m4a
		cvtmsg "$f" "$g"
		unogg "$f" 
		toaac tempo${BASHPID}.wav "$1"/"$g"  
	  fi
	  if [ $tofmt == alac ]
      	  then
	  	g=`basename "$f" .ogg`.m4a
		cvtmsg "$f" "$g"
	  	unogg "$f"
		toalac tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == flac ]
      	  then
		g=`basename "$f" .ogg`.flac
		cvtmsg "$f" "$g"
		toflac "$f" "$1"/"$g"
	  fi
	  if [ $tofmt == vbr ]
      	  then
	  	g=`basename "$f" .ogg`.mp3
	  	cvtmsg "$f" "$g"
	  	unogg "$f" 
		tomp3 "tempo${BASHPID}.wav" "$1"/"$g" "$VBRENC_OPTS" 
	  fi
	  if [ $tofmt == cbr ]
      	  then
	  	g=`basename "$f" .ogg`.mp3
	  	cvtmsg "$f" "$g"
	  	unogg "$f" 
		tomp3 "tempo${BASHPID}.wav" "$1"/"$g" "$CBRENC_OPTS" 
	  fi
	  if [ $tofmt == wav ]
      	  then
	  	g=`basename "$f" .ogg`.wav
	  	cvtmsg "$f" "$g"
	  	unogg "$f" 
		towav "tempo${BASHPID}.wav" "$1"/"$g"
	  fi
	  rm -f tempo${BASHPID}.wav 
	  notify_parent ) &	  
      fi
    done
    for f in *.flac
    do
      if [ "$f" != '*.flac' ]
      then
	  child_start  
	  ( flactag "$f"
	  if [ $tofmt == ogg ]
	  then
		g=`basename "$f" .flac`.ogg
		cvtmsg "$f" "$g"
		if autodecode_flac2ogg "$f" "$1"/"$g" 
		then true 
		else unflac "$f"; toogg tempo${BASHPID}.wav "$1"/"$g"
		fi
      	  fi
	  if [ $tofmt == aac ]
      	  then
	  	g=`basename "$f" .flac`.m4a
		cvtmsg "$f" "$g"
		unflac "$f" 
		toaac tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == alac ]
      	  then
	  	g=`basename "$f" .flac`.m4a
		cvtmsg "$f" "$g"
	  	unflac "$f"
		toalac tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == flac ]
      	  then
		copymsg "$f"
		cp "$f" "$dstdir"/"$1"/"$f"
	  fi
	  if [ $tofmt == wav ]
      	  then
	  	g=`basename "$f" .flac`.wav
		cvtmsg "$f" "$g"
	  	unflac "$f"
		towav tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == vbr ]
      	  then
	  	g=`basename "$f" .flac`.mp3
	  	cvtmsg "$f" "$g"
	  	unflac "$f" 
		tomp3 tempo${BASHPID}.wav "$1"/"$g" "$VBRENC_OPTS"
	  fi
	  if [ $tofmt == cbr ]
      	  then
	  	g=`basename "$f" .flac`.mp3
	  	cvtmsg "$f" "$g"
	  	unflac "$f" 
		tomp3 tempo${BASHPID}.wav "$1"/"$g" "$CBRENC_OPTS"
	  fi
	  rm -f tempo${BASHPID}.wav 
	  notify_parent ) &	  
      fi
    done
    for f in *.m4a *.mp4 *.aac
    do
      if [ "$f" != '*.m4a' -a "$f" != '*.mp4' -a "$f" != '*.aac' ]
      then
	  child_start  
	  ( mp4tag "$f"	
	  if [ $tofmt == ogg ]
	  then
		g=${f%%.*}.ogg
		cvtmsg "$f" "$g"
		unmp4 "$f"
		toogg tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == aac ]
      	  then
		if [ $mp4type != alac ]
		then
			copymsg "$f" 
	  		cp "$f" "$dstdir"/"$1"/"$f"
		else
			g="$f"
			cvtmsg "$f" "$g"
			unalac "$f"
			toaac tempo${BASHPID}.wav "$1"/"$g"
		fi
	  fi
	  if [ $tofmt == alac ]
      	  then
		if [ "$mp4type" == alac ]
		then
			copymsg "$f" 
	  		cp "$f" "$dstdir"/"$1"/"$f"
		else
			g="$f"
			cvtmsg "$f" "$g"
			unaac "$f"
			toalac tempo${BASHPID}.wav "$1"/"$g"
		fi
	  fi
	  if [ $tofmt == flac ]
      	  then
		g=${f%%.*}.flac
		cvtmsg "$f" "$g"
		unmp4 "$f"
		toflac tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == vbr ]
      	  then
  	  	g=${f%%.*}.mp3
  	  	cvtmsg "$f" "$g"
	  	unmp4 "$f"
		tomp3 tempo${BASHPID}.wav "$1"/"$g" "$VBRENC_OPTS"
	  fi
	  if [ $tofmt == cbr ]
      	  then
  	  	g=${f%%.*}.mp3
  	  	cvtmsg "$f" "$g"
	  	unmp4 "$f"
		tomp3 tempo${BASHPID}.wav "$1"/"$g" "$CBRENC_OPTS"
	  fi
	  if [ $tofmt == wav ]
      	  then
	  	g=${f%%.*}.wav
	  	cvtmsg "$f" "$g"
	  	unmp4 "$f" 
		towav "tempo${BASHPID}.wav" "$1"/"$g"  
	  fi	
	  rm -f tempo${BASHPID}.wav 
	  notify_parent ) &	    
      fi
    done
    for f in *.mp3
    do 
      if [ "$f" != '*.mp3' ]
      then
	  child_start  
	  ( mp3tag "$f"
	  if [ $tofmt == ogg ]
	  then
	  	g=`basename "$f" .mp3`.ogg
		cvtmsg "$f" "$g"
		unmp3 "$f"
		toogg tempo${BASHPID}.wav "$1"/"$g"
	  fi
  	  if [ $tofmt == flac ]
      	  then
		g=`basename "$f" .mp3`.flac
		cvtmsg "$f" "$g"
		unmp3 "$f"
		toflac tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == aac ]
      	  then
	  	g=`basename "$f" .mp3`.m4a
		cvtmsg "$f" "$g"
	  	unmp3 "$f"
		toaac tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == alac ]
      	  then
	  	g=`basename "$f" .mp3`.m4a
		cvtmsg "$f" "$g"
	  	unmp3 "$f"
		toalac tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == wav ]
      	  then
	  	g=`basename "$f" .mp3`.wav
		cvtmsg "$f" "$g"
	  	unmp3 "$f"
		towav tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == vbr ]
      	  then
	  	rate=`mp3info -x -p "%r" "$f"`
	  	cvtmsg "$f" "$g"
	  	if [ $rate -eq Variable ]
	  	then
	      		cp "$f" "$dstdir"/"$1"
	  	else
			tomp3 "$f" "$1"/"$f" "$VBRENC_OPTS"
	  	fi
	  fi
	  if [ $tofmt == cbr ]
      	  then
	  	rate=`mp3info -x -p "%r" "$f"`
	  	cvtmsg "$f" "$g"
	  	if [ $rate = 128 ]
	  	then
	      		cp "$f" "$dstdir"/"$1"
	  	else
			tomp3 "$f" "$1"/"$f" "$CBRENC_OPTS"
	  	fi
	  fi
	  rm -f tempo${BASHPID}.wav 
	  notify_parent ) &	  
       fi
    done
    for f in *.wma
    do
      if [ "$f" != '*.wma' ]
      then
	  child_start  
	  ( wmatag "$f"
	  if [ $tofmt == ogg ]
	  then
		g=`basename "$f" .wma`.ogg
		cvtmsg "$f" "$g"
		unwma "$f" 
		toogg tempo${BASHPID}.wav "$1"/"$g"
      	  fi
	  if [ $tofmt == aac ]
      	  then
		g=`basename "$f" .wma`.m4a
		cvtmsg "$f" "$g"
		unwma "$f" 
		toaac tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == alac ]
      	  then
	  	g=`basename "$f" .wma`.m4a
		cvtmsg "$f" "$g"
	  	unwma "$f"
		toalac tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == flac ]
      	  then
		g=`basename "$f" .wma`.flac
		cvtmsg "$f" "$g"
	  	unwma "$f"
		toflac tempo${BASHPID}.wav "$1"/"$g"
	  fi
	  if [ $tofmt == vbr ]
      	  then
	  	g=`basename "$f" .wma`.mp3
	  	cvtmsg "$f" "$g"
		set -x
	  	unwma "$f" 
		tomp3 "tempo${BASHPID}.wav" "$1"/"$g" "$VBRENC_OPTS"
	  fi
	  if [ $tofmt == cbr ]
      	  then
	  	g=`basename "$f" .wma`.mp3
	  	cvtmsg "$f" "$g"
	  	unwma "$f" 
		tomp3 "tempo.wav" "$1"/"$g" "$CBRENC_OPTS" 
	  fi
	  if [ $tofmt == wav ]
      	  then
	  	g=`basename "$f" .wma`.wav
	  	cvtmsg "$f" "$g"
	  	unwma "$f" 
		towav "tempo${BASHPID}.wav" "$1"/"$g"  
	  fi	 
	  rm -f tempo${BASHPID}.wav 
	  notify_parent ) & 
      fi
    done
    for f in *.wav
    do 
      if [ "$f" != '*.wav' ]
      then
	  child_start  
	  ( wavtag "$f"
	  if [ $tofmt == ogg ]
	  then
	  	g=`basename "$f" .wav`.ogg
		cvtmsg "$f" "$g"
		toogg "$f" "$1"/"$g"
	  fi
  	  if [ $tofmt == flac ]
      	  then
		g=`basename "$f" .wav`.flac
		cvtmsg "$f" "$g"
		toflac "$f" "$1"/"$g"
	  fi
	  if [ $tofmt == aac ]
      	  then
	  	g=`basename "$f" .wav`.m4a
		cvtmsg "$f" "$g"
		toaac "$f" "$1"/"$g"
	  fi
	  if [ $tofmt == alac ]
      	  then
	  	g=`basename "$f" .wav`.m4a
		cvtmsg "$f" "$g"
	  	unmp3 "$f"
		toalac "$f" "$1"/"$g"
	  fi
	  if [ $tofmt == wav ]
      	  then
		copymsg "$f" 
		cp "$f" "$dstdir"/"$1"
	  fi
	  if [ $tofmt == vbr  ]
      	  then
		g=`basename "$f" .wav`.mp3
	  	cvtmsg "$f" "$g"
		tomp3 "$f" "$1"/"$f" "$VBRENC_OPTS"
	  fi
	  if [ $tofmt == cbr ]
      	  then
		g=`basename "$f" .wav`.mp3
	  	cvtmsg "$f" "$g"
		tomp3 "$f" "$1"/"$g" "$CBRENC_OPTS"
	  fi
	  notify_parent ) &	  
       fi
    done
}

# This does not have to be reentrant, it is run only from the main thread
child_start() {
	while true
	do		
		if [ $avail_proc -eq 0 ]
		then
			sleep 2
		else
			break
		fi
	done
	avail_proc=$((avail_proc - 1))
}

# run from trap (USR1)
child_finished() {
	# we can't just increment avail_proc as there can be several signals merged. 
	children=`jobs -p`	
	running=`ps -o pid='' $children | wc -l` # these ps args are FreeBSD compatible
	avail_proc=$((MAX_PROC - running))
}

# generate trap
notify_parent() {
	kill -USR1 $$ 2>/dev/null
}

do_multiproc() {
	[ $MAX_PROC -ne 0 ]
}

if do_multiproc
then 
	avail_proc=$MAX_PROC		#number of available threads
	trap child_finished USR1
else
	echo 0 is not \(yet \?\) a suitable value for MAX_PROC in ${BASH_SOURCE[0]}.
	echo Set MAX_PROC to at least 1 and report your possible need for MAX_PROC=0 to the author.
	exit
fi

if [ $# -ge 1 ] 
then
	tofmt=$1
	if [ \( $1 != "ogg" \) -a \( $1 != "vbr" \)  -a \( $1 != "cbr" \) -a \( $1 != "flac" \) -a \( $1 != "aac" \) -a \( $1 != "wav" \) -a \( $1 != "alac" \) ] 
	then 
		help 	  
		exit
	fi
else
	tofmt=ogg
fi

top="$PWD"
if [ $# -ge 2 ] 
then
	cd "$2"
	srcdir="$PWD"
	cd "$PWD"
else
	srcdir="$top"/src
fi
cd "$top"
if [ $# -ge 3 ] 
then
	mkdir -p "$3"
	cd "$3"
	dstdir="$PWD"
	cd "$PWD"
else
	dstdir="$top"/dst
fi

if [ ! -d "$srcdir" ] 
then
	help
	exit
fi

cd "$srcdir"
dupdir .
cd "$srcdir"
cvtdir .
children=`jobs -p`	
wait $children

#http://mediainfo.sourceforge.net
#http://www.gnu.org/software/libextractor/
#http://linuxfr.org/~haypo/23207.html
