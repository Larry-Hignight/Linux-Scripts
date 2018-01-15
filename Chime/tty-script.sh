#!/bin/sh

# In order for this script to work, set NAutoVTs=1 in the /etc/systemd/logind.conf file
#Consolse and Shellworld on TTY1
openvt -u -l -f -c 2   #check current time
openvt -u -l -f -c 3   #sometimes ncftp
openvt -u -l -f -c 4   #playing mp3s
openvt -u -l -f -c 5   #w or who
openvt -u -l -f -c 6   #lynx www.dxworld.com/tvfmlog.html
openvt -u -l -f -c 13  #sitting in videos/tv-news
openvt -u -l -f -c 18  #playing videos/music
openvt -u -l -f -c 23  #playing video streams with xvfb-run

# These TTYs don't have a predefined role
openvt -u -l -c 7 
openvt -u -l -c 8
openvt -u -l -c 9
openvt -u -l -c 10
openvt -u -l -c 11
openvt -u -l -c 12
openvt -u -l -c 14
openvt -u -l -c 15
openvt -u -l -c 16
openvt -u -l -c 17
openvt -u -l -c 19
openvt -u -l -c 20
openvt -u -l -c 21
openvt -u -l -c 22
openvt -u -l -c 24




#chvt 4 && cd mp3
#chvt 13 && cd videos/tv-news
#chvt 18 && cd videos/music
#chvt 1



#vt-log - console error log
#vt-sw - Shellworld
#vt-time - check current time
#vt-ftp - sometimes ncftp
#vt-mp3 - playing mp3s
#vt-who - w or who
#vt-tv - lynx www.dxworld.com/tvfmlog.html
#vt-news - sitting in videos/tv-news
#vt-music - playing videos/music
#vt-bin - sitting in /home/chime/bin
#vt-streams - playing video streams with xvfb-run
