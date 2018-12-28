

## ------------------------------------------------------------------------------------------------------------
## This section is added during the post-install process


##  Controls TTY behavior -------------------------------------------------------------------------------------
set complete = enhance
     complete sudo 'n/-l/u/' 'p/1/c/'

set autoexpand
set autologout=n


## Old aliases that Dallas created for Chime ------------------------------------------------------------------

alias ls              "ls -1"
alias frm	      "frm|sort"
alias fftp            "/usr/bin/ftp -i"
alias ftp	       $HOME/bin/ftp
alias kwota            $home/bin/kwota
alias lynx	      "lynx -show_cursor"
alias trn              rn
alias zip	      "zip -v"
alias fm              "lynx -show_cursor -cfg /home/chime/lynx.cfg"
alias pilot            pilot -g
alias cddb            "abcde -a cddb"
alias newslite        "newslite -a gn319759 A9761212 -p 119 -s news.giganews.com"
alias nuke-fakes      "rm -f .*MP5*"
alias ff              "xvfb-run firefox"
alias rp              "xvfb-run -a -s '-screen 0 1024x768x16' /opt/real/RealPlayer/realplay "
alias df              "df  -h "
alias start-privoxy   "sudo /etc/init.d/privoxy restart"
alias reload-dectalk  "/usr/bin/sudo /sbin/modprobe  -r speakup_dectlk; /usr/bin/sudo /sbin/modprobe  speakup_dectlk "
alias rd              "/usr/bin/sudo /sbin/modprobe  -r speakup_dectlk; /usr/bin/sudo /sbin/modprobe  speakup_dectlk "
alias type            "more"
alias time            "date +'%A %B %d %r %Z %Y'"


## Run local scripts ------------------------------------------------------------------------------------------

alias startup         "sh /home/chime/linux-scripts/chime-startup.sh"
alias cap             "sh /home/chime/linux-scripts/cap"
alias inflection      "sudo sh /home/chime/linux-scripts/inflection.sh"
alias jjs             "sh /home/chime/linux-scripts/jjs"
alias rn              "sh /home/chime/linux-scripts/rn"
alias space2dash      "python /home/chime/linux-scripts/space2dash.py"
alias strm2mp3        "sh /home/chime/linux-scripts/strm2mp3"
alias thpodder        "sh /home/chime/linux-scripts/thpodder"
alias weather         "sh /home/chime/linux-scripts/weather.sh"
alias wx              "sh /home/chime/linux-scripts/wx"
alias sw              "ssh -p 845 server2.shellworld.net"


## Sound card and cdrom related aliases  ----------------------------------------------------------------------

alias mpv0     "mpv --really-quiet --no-video --audio-device=alsa/plughw:CARD=Intel,DEV=0"
alias mpv1     "mpv --really-quiet --no-video --audio-device=alsa/plughw:CARD=AudioPCI,DEV=0"
alias mpv2     "mpv --really-quiet --no-video --audio-device=alsa/plughw:CARD=DGX,DEV=0"
alias mpv3     "mpv --really-quiet --no-video --audio-device=alsa/plughw:CARD=Audigy2,DEV=0"

alias sound0   "alsamixer -c 0"
alias sound1   "alsamixer -c 2"
alias sound2   "alsamixer -c 1"
alias sound3   "alsamixer -c 3"

alias cd-rom0 "sh /home/chime/linux-scripts/cd-rom.sh 1"
alias cd-rom1 "sh /home/chime/linux-scripts/cd-rom.sh 2"
alias cd-rom2 "sh /home/chime/linux-scripts/cd-rom.sh 0"
alias cd-rom3 "sh /home/chime/linux-scripts/cd-rom.sh 3"


## Audio program aliases --------------------------------------------------------------------------------------

alias mp3l     "/home/chime/linux-scripts/mp3l.sh"
alias mdae     "/home/chime/linux-scripts/mdae.sh"
alias mp3quiet "/home/chime/linux-scripts/mp3quiet.sh"
alias mp3n     "python /home/chime/linux-scripts/mp3-rename.py"
alias mp4tomp3 "python /home/chime/linux-scripts/mp42mp3.py"


## Screen aliases ---------------------------------------------------------------------------------------------

alias sffr "screen -S"
alias rffr "screen -d -R"


## Misc aliases -----------------------------------------------------------------------------------------------

alias bellw         "ssh chime@192.168.0.2 w"
alias d-record      "/home/chime/docker_record/new/docker_record.sh"
alias del           "/bin/rm -f"
alias ffr           "python /home/chime/linux-scripts/ffr.py"
alias lltag         "lltag --rename '%a-%t' --yes"
alias nano          "nano -tzxk"
alias pod-search    "python /home/chime/linux-scripts/pod-search.py"
alias rcbs          "/home/chime/linux-scripts/rcbs.sh"
alias rvlc          "/home/chime/linux-scripts/rvlc.sh"
alias safe-rm       "python /home/chime/linux-scripts/rm.py"
alias sd-card       "/home/chime/linux-scripts/automount_sd_only.sh"
alias snip-art      "sh /home/chime/linux-scripts/mp3_image_removal.sh"
alias src           "source /etc/csh.cshrc"
alias sudo='A=`alias` sudo '
alias wftp          "/home/chime/linux-scripts/ftp-get.sh"


## Chime-OCR aliases ------------------------------------------------------------------------------------------

alias ocr   "python3 ~/Chime-OCR/ocr.py"
