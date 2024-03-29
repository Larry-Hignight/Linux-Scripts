## New command-line tools
##
## This file contains installation instructions for several new and interesting Linux CLI tools.
## Information for most of the tools listed below comes from the websites:
## https://remysharp.com/2018/08/23/cli-improved
## https://hackaday.com/2018/08/29/linux-fu-modernize-your-command-line/
##
## TODO
## Oh-My-Zsh (https://medium.com/wearetheledger/oh-my-zsh-made-for-cli-lovers-installation-guide-3131ca5491fb)
## Highlight (http://www.andre-simon.de/doku/highlight/en/highlight.php)
## noti (https://github.com/variadico/noti)
## csvkit (https://csvkit.readthedocs.io/en/1.0.3/)
## entr (http://www.entrproject.org/)
## diff-so-fancy (https://github.com/so-fancy/diff-so-fancy)
## ack (apt-get install ack-grep), ag, jq
## multitail looks promising (apt-get install multitail)


#######################################
## CAT like programs: tac, bat, ccat ##
#######################################

# tac is cat w/ reverse file output and is installed by default in Ubuntu
# Example: tac filename


# bat is similar to cat w/ color syntax highlighting, line numbers, git integration and "file decorations".
# Github: https://github.com/sharkdp/bat
wget https://github.com/sharkdp/bat/releases/download/v0.6.0/bat_0.6.0_amd64.deb
sudo dpkg -i bat_0.6.0_amd64.deb


# ccat is the colorizing cat.
# Github: https://github.com/jingweno/ccat
#
# I haven't had an opportunity to install this app


############################
## DU like programs: ncdu ##
############################

# ncdu is a disk usage analyzer w/ a nice ncurses TUI
# This tool is incredibly useful and the interface makes it easy to find where disk space is being used
# Website:  https://dev.yorhel.nl/ncdu
sudo apt-get install ncdu

# nnn is a terminal based file manager
# Github: https://github.com/jarun/nnn
sudo apt-get install nnn

############################
## FIND like programs: fd ##
############################

# fd is an "opinionated" alternative to the find command
# The author states that it is intended to cover the most common 80% of find use-cases
# Github: https://github.com/sharkdp/fd/

# This is a pretty cool program that I want to continue using in a container before installing on my host system
# The --exec and multiple threads are very nice features
wget https://github.com/sharkdp/fd/releases/download/v7.1.0/fd_7.1.0_amd64.deb
dpkg -i fd_7.1.0_amd64.deb 


#######################################################
## PING like programs: mtr, oping/noping, prettyping ##
#######################################################

# mtr is a combination of ping and traceroute
# The default behavior is a real-time traceroute TUI
# The --report flag prints to stdout similar to traceroute
# On systems w/ X installed, mtr will open a GUI (use the -t flag for terminal output)
sudo apt-get install mtr

# oping is similar to ping, but it can ping multiple hosts in parallel
# noping is oping plus a nice ncurses TUI
# Unfortunatly, the default Ubuntu oping package requires sudo to run correctly AND it doesn't seem to be able to ping multiple hosts
sudo apt-get install oping

# prettyping is a TUI for ping
# Like oping/noping, the output is pretty cool, but the installation and setup just aren't worth the effort outside of a demo
# I don't see much value in using either prettyping or oping.  While noping is pretty cool, if it doesn't hose your terminal.


######################################
## TOP like programs: glances, nmon ##
######################################

# nmon is interesting because it monitors different parts of the system than htop and glances
# Website: https://haydenjames.io/alternatives-top-htop/
sudo apt-get install nmon

# glances is a very good looking system monitor (w/ a dark background) that also has a section for monitoring containers
# Website: https://haydenjames.io/alternatives-top-htop/
sudo apt-get install glances
sudo -H pip install --upgrade glances
