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
alias ls "ls -1"
alias frm	'frm|sort'
alias fftp   '/usr/bin/ftp -i'
alias ftp	$HOME/bin/ftp
alias kwota   '$home/bin/kwota'
alias lynx	'lynx -show_cursor -cfg ~/usr/bin/lynx.cfg'
# rn's alias was getting too complicated, so now it's a shell script in ~/bin
#alias trn	'trn -h +hfrom'
alias trn	rn
alias zip	'zip -v'
alias fm 'lynx -cfg ~/usr/bin/lynx.cfg'
#alias lynx lynx -cfg ~/lynx.cfg
alias pilot pilot -g
alias cddb 'abcde -a cddb'
alias newslite 'newslite -a gn319759 A9761212 -p 119 -s news.giganews.com'
alias nuke-fakes 'rm -f .*MP5*'
setenv DISPLAY ':99'
# no longer needed in Debian, slightly different Xvfb setup
#  alias ff 'firefox  --display=:99  -P nullvideo  '
# alias ff 'xvfb-run firefox -no-remote -a nullvideo -P nullvideo '
alias ff "xvfb-run -a -s '-screen 0 640x480x16' firefox -no-remote -a nullvideo -P nullvideo "
alias rp "xvfb-run -a -s '-screen 0 1024x768x16' /opt/real/RealPlayer/realplay "
alias df 'df  -h '
alias start-privoxy 'sudo /etc/init.d/privoxy restart'
#alias reload-dectalk modprobe -r speakup_dectlk: modprobe dectlk
# alias smpdectlk='/usr/bin/sudo /sbin/modprobe '
#  smpdectlk = Sudo ModProbe  4 dectlk
# alias reload-dectalk 'smpdectlk -r speakup_dectlk; smpdectlk speakup_dectlk '
alias reload-dectalk '/usr/bin/sudo /sbin/modprobe  -r speakup_dectlk; /usr/bin/sudo /sbin/modprobe  speakup_dectlk '
alias rd '/usr/bin/sudo /sbin/modprobe  -r speakup_dectlk; /usr/bin/sudo /sbin/modprobe  speakup_dectlk '
alias ls 'ls -1'
alias type 'less'
#alias date +'%A %B %d %r %Z %Y'
