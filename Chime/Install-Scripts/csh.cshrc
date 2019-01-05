

## ------------------------------------------------------------------------------------------------------------
## This section is added during the post-install process


##  Controls TTY behavior -------------------------------------------------------------------------------------

set complete = enhance
     complete sudo 'n/-l/u/' 'p/1/c/'

set autoexpand
set autologout=n

dmesg -D
stty rows 1999 cols 340

alias sudo='A=`alias` sudo '


## Basic command aliases --------------------------------------------------------------------------------------

alias bellw           "ssh chime@192.168.0.2 w"
alias del             "/bin/rm -f"
alias df              "df  -h "
alias ls              "ls -1"
alias kwota           "du -hs"
alias nano            "nano -tzxk"
alias src             "source /etc/csh.cshrc"
alias time            "date +'%A %B %d %r %Z %Y'"
alias zip	      "zip -v"


## Shellworld aliases -----------------------------------------------------------------------------------------

alias fm              "lynx -cfg /home/chime/lynx.cfg"
alias frm	      "frm | sort"
alias start-privoxy   "sudo /etc/init.d/privoxy restart"
alias sw              "ssh -p 845 server2.shellworld.net"


## Local script aliases ---------------------------------------------------------------------------------------

alias ffr             "python /usr/local/bin/ffr.py"
alias inflection      "sudo sh /usr/local/bin/inflection.sh"
alias jjs             "sh /usr/local/bin/jjs"
alias pod-search      "python /usr/local/bin/pod-search.py"
alias rn              "sh /usr/local/bin/rn"
alias rw              "Rscript /usr/local/bin/remote-w.R"
alias sd-card         "/usr/local/bin/automount_sd_only.sh"
alias space2dash      "python /usr/local/bin/space2dash.py"
alias startup         "sh /usr/local/bin/chime-startup.sh"
alias thpodder        "sh /usr/local/bin/thpodder"
alias wftp            "/usr/local/bin/ftp-get.sh"
alias wx              "sh /usr/local/bin/wx"


## Old aliases that Dallas created for Chime ------------------------------------------------------------------

alias cddb            "abcde -a cddb"
alias ff              "xvfb-run firefox"
alias rd              "/usr/bin/sudo /sbin/modprobe  -r speakup_dectlk; /usr/bin/sudo /sbin/modprobe  speakup_dectlk "


## Sound card and cdrom related aliases  ----------------------------------------------------------------------

alias mpv0            "mpv --really-quiet --no-video --audio-device=alsa/plughw:CARD=Intel,DEV=0"
alias mpv1            "mpv --really-quiet --no-video --audio-device=alsa/plughw:CARD=AudioPCI,DEV=0"
alias mpv2            "mpv --really-quiet --no-video --audio-device=alsa/plughw:CARD=DGX,DEV=0"
alias mpv3            "mpv --really-quiet --no-video --audio-device=alsa/plughw:CARD=Audigy2,DEV=0"

# TODO - This needs to be changed to use the card name, not device number
alias sound0          "alsamixer -c 0"
alias sound1          "alsamixer -c 2"
alias sound2          "alsamixer -c 1"
alias sound3          "alsamixer -c 3"

# TODO - This needs to be tested
alias cd-rom0         "sh /usr/local/bin/cd-rom.sh 1"
alias cd-rom1         "sh /usr/local/bin/cd-rom.sh 2"
alias cd-rom2         "sh /usr/local/bin/cd-rom.sh 0"
alias cd-rom3         "sh /usr/local/bin/cd-rom.sh 3"


## Audio program aliases --------------------------------------------------------------------------------------

alias lltag           "lltag --rename '%a-%t' --yes"
alias mp3l            "/usr/local/bin/mp3l.sh"                     # Length in hours minutes and seconds of an mp3 file
alias mdae            "/usr/local/bin/mdae.sh"
alias mp3quiet        "/usr/local/bin/mp3quiet.sh"
alias mp3n            "python /usr/local/bin/mp3-rename.py"        # TODO - What does this program do...
alias mp4tomp3        "python /usr/local/bin/mp42mp3.py"
alias snip-art        "sh /usr/local/bin/mp3_image_removal.sh"


## Chime-OCR aliases ------------------------------------------------------------------------------------------

alias ocr             "python3 /usr/local/Chime-OCR/ocr.py"


## Old aliases ------------------------------------------------------------------------------------------------

#alias cap            "sh /usr/local/bin/cap"
#alias d-record       "/usr/local/docker_record/new/docker_record.sh"
#alias pilot          "pilot -g"
#alias rcbs           "/usr/local/bin/rcbs.sh"
#alias rvlc           "/usr/local/bin/rvlc.sh"
#alias safe-rm        "python /usr/local/bin/rm.py"
#alias type           "more"
#alias weather        "sh /usr/local/bin/weather.sh"
