# /etc/csh.cshrc: system-wide .cshrc file for csh(1) and tcsh(1)

if ($?tcsh && $?prompt) then

	bindkey "\e[1~" beginning-of-line # Home
	bindkey "\e[7~" beginning-of-line # Home rxvt
	bindkey "\e[2~" overwrite-mode    # Ins
	bindkey "\e[3~" delete-char       # Delete
	bindkey "\e[4~" end-of-line       # End
	bindkey "\e[8~" end-of-line       # End rxvt

	set autoexpand
	set autolist
	set prompt = "%U%m%u:%B%~%b%# "
endif
unset autologout

# Old aliases that Dallas created for Chime
alias ls              "ls -1"
alias frm	      "frm|sort"
alias fftp            "/usr/bin/ftp -i"
alias ftp	       $HOME/bin/ftp
alias kwota           "$home/bin/kwota"
alias lynx	      "lynx -show_cursor -cfg ~/usr/bin/lynx.cfg"
# rn's alias was getting too complicated, so now it's a shell script in ~/bin
#alias trn	      "trn -h +hfrom"
alias trn	       rn
alias zip	      "zip -v"
alias fm              "lynx -cfg ~/usr/bin/lynx.cfg"
#alias lynx           "lynx -cfg ~/lynx.cfg"
alias pilot            pilot -g
alias cddb            "abcde -a cddb"
alias newslite        "newslite -a gn319759 A9761212 -p 119 -s news.giganews.com"
alias nuke-fakes      "rm -f .*MP5*"
alias ff              "xvfb-run -a -s '-screen 0 640x480x16' firefox -no-remote -a nullvideo -P nullvideo "
alias rp              "xvfb-run -a -s '-screen 0 1024x768x16' /opt/real/RealPlayer/realplay "
alias df              "df  -h "
alias start-privoxy   "sudo /etc/init.d/privoxy restart"
alias reload-dectalk  "/usr/bin/sudo /sbin/modprobe  -r speakup_dectlk; /usr/bin/sudo /sbin/modprobe  speakup_dectlk "
alias rd              "/usr/bin/sudo /sbin/modprobe  -r speakup_dectlk; /usr/bin/sudo /sbin/modprobe  speakup_dectlk "
alias type            "less"
#alias date            +'%A %B %d %r %Z %Y'

# Section for running local scripts
alias startup         "sh ~/Linux-Scripts/Chime/chime-startup.sh"
alias mount-bell      "sh ~/Linux-Scripts/Chime/mount-bell.sh"
alias tty-script      "sh ~/Linux-Scripts/Chime/tty-script.sh"
alias unmount-bell    "sh ~/Linux-Scripts/Chime/unmount-bell.sh"
alias cap             "sh ~/Linux-Scripts/Chime/Old-Scripts/cap"
alias inflection      "sudo sh ~/Linux-Scripts/Chime/Old-Scripts/inflection.sh"
alias jjs             "sh ~/Linux-Scripts/Chime/Old-Scripts/jjs"
alias rn              "sh ~/Linux-Scripts/Chime/Old-Scripts/rn"
alias space2dash      "sh ~/Linux-Scripts/Chime/Old-Scripts/space2dash"
alias strm2mp3        "sh ~/Linux-Scripts/Chime/Old-Scripts/strm2mp3"
alias thpodder        "sh ~/Linux-Scripts/Chime/Old-Scripts/thpodder"
alias weather         "sh ~/Linux-Scripts/Chime/Old-Scripts/weather.sh"
alias wx              "sh ~/Linux-Scripts/Chime/Old-Scripts/wx"
