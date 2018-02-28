#!/bin/bash
#  The following script uses lynx to retrieve the 
#  weather for the entered zipcode or city/state.
#  A simple awk program selects the fields to be displayed.
#  parse options using the getopts shell build-in.
while getopts "wdsnyf" NAME; do
  #echo "$NAME"
  #echo $OPTIND
  case $NAME in 
    "w") WFLAG="w";;
    "d") DFLAG="d";;
    "s") SFLAG="s";;
    "n") NFLAG="n";;
    "y") YFLAG="y";;
    "f") FFLAG="f";;
  esac
done
#echo "$# ${#WFLAG}"
if [ $((${#WFLAG} + ${#DFLAG} + ${#SFLAG}+ ${#NFLAG} + ${#YFLAG} + ${#FFLAG})) -gt 0 ]; then
  shift
fi
FLAGS="$WFLAG$DFLAG$SFLAG$NFLAG$YFLAG$FFLAG"
#echo $FLAGS
case $# in
    0) echo "Usage:"
       echo "    wx options <zipcode>"
       echo "  or" 
       echo "    wx options <city>,  <state>"
       echo "    where options are:"
       echo "    -w wind a dnwindchill"
       echo "    -d dewpoint and humidity"
       echo "    -s sunrise and sunset"
       echo "    -n normal high and low temp"
       echo "    -y yesterday\'s temps and degree days"
       echo "    -f forecast for the next seven days"
       echo "Example:  wx -dws boston, ma" 
       
       exit;;
    1) query=$1;;
    2) query=${1/%,}%2C+$2\";;
    3) query=${1}+${2/%,}%2C+$3\";;
esac
lynx -dump  http://braille.wunderground.com/cgi-bin/findweather/getForecast?query=${query}|\
awk -v flags=$FLAGS ' \
BEGIN {
  wflag = index(flags, "w")
  dflag = index(flags, "d")
  sflag = index(flags, "s")
  nflag = index(flags, "n")
  yflag = index(flags, "y")
  fflag = index(flags, "f")
  #print sflag
}
/^ *Observed/ { line = ""
               #print NF    
               fflag = 0
               sub(" *" $1 " +" $2 " +", "")
               line = line  $0 
               gsub(/,/, "", line)
               gsub(/$/, ",", line)
              } 
/^ *Temperature/ {t = $2
    #print NF  
    sub(/..$/, "", t)
    line = line " " t  
    }
/^ *Conditions/ {
           #print NF
           sub(" *" $1 " +", "")
           line = line " " $0
         }
/^ *Windchill/ {
  if (wflag > 0) {
    t = $2
    sub(/..$/, "", t)
    wline = $1 " " t
  }
}
/^ *Wind/ {
  if (wflag > 0) {
    direction = $2
    gsub(/N/, "North", direction)
    gsub(/E/, "East", direction)
    gsub(/S/, "South", direction)
    gsub(/W/, "West", direction)
    wline1 = $1 " " direction " " $3 " " $4 " " $5
  }
}
/^ *Dew/ {
  if (dflag > 0) {
    t = $3
    sub(/..$/, "", t)
    dline = $1 " " $2 " " t
  }
}
/^ *Humidity/ {
  if (dflag > 0) {
    dline1 = $1 " " $2
  }
}
/^ *Sunrise/ {
  if (sflag > 0) {
    sline = $1 " " $2 " " $3
  }
}
/^ *Sunset/ {
  if (sflag > 0) {
    sline1 = $1 " " $2 " " $3
  }
}
/^ *Normal high/ {
  if (nflag > 0) {
    t = $3
    sub(/..$/, "", t)
    nline1 = $1 " " $2 " " t
  }
}
/^ *Normal low/ {
  if (nflag > 0) {
    t = $3
    sub(/..$/, "", t)
    nline2 = $1 " " $2 " " t
  }
}
/^ *Record high/ {
  if (nflag > 0) {
    t = $3
    sub(/..$/, "", t)
    nline3 = $1 " " $2 " " t " in " $4
  }
}
/^ *Record low/ {
  if (nflag > 0) {
    t = $3
    sub(/..$/, "", t)
    nline4 = $1 " " $2 " " t " in " $5
  }
}
/^ *Yesterday.. Maximum/ {
  if (yflag > 0) {
    t = $3
    sub(/..$/, "", t)
    yline1 = $1 " " $2 " " t
  }
}
/^ *Yesterday.. Minimum/ {
  if (yflag > 0) {
    t = $3
    sub(/..$/, "", t)
    yline2 = $1 " " $2 " " t
  }
}
/^ *Yesterday.. Heating/ {
  if (yflag > 0) {
    yline3 = $0
  }
}
/^ *Yesterday.. Cooling/ {
  if (yflag > 0) {
    yline4 = $0
  }
}
{ if (fflag > 0) { print } }
END {
  print line 
  if (wflag > 0) { print wline1; print wline }
  if (dflag > 0) { print dline; print dline1 }
  if (sflag > 0) { print sline; print sline1 }
  if (nflag > 0) { print nline1; print nline2; print nline3; print nline4 }
  if (yflag > 0) {
    print yline1
    print yline2
    print yline3
    print yline4
  }
}' 
