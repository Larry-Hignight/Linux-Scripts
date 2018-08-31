## New command-line tools
##
## This file contains installation instructions for several new and interesting Linux CLI tools.
## Information for most of the tools listed below comes from the websites:
## https://remysharp.com/2018/08/23/cli-improved
## https://hackaday.com/2018/08/29/linux-fu-modernize-your-command-line/
##


##################################
## CAT like programs: bat, ccat ##
##################################

# bat is similar to cat w/ color syntax highlighting, line numbers, git integration and "file decorations".
# Github: https://github.com/sharkdp/bat
wget https://github.com/sharkdp/bat/releases/download/v0.6.0/bat_0.6.0_amd64.deb
sudo dpkg -i bat_0.6.0_amd64.deb


# ccat is the colorizing cat.
# Github: https://github.com/jingweno/ccat
#
# I haven't had an opportunity to install this app


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
