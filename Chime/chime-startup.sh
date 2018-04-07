echo "running the inflection script" && echo "root-beer" | sudo -S sh /home/chime/Linux-Scripts/Chime/inflection.sh && echo "successfull"

echo "loading pcspkr module" && sudo modprobe pcspkr && echo "successfull"

echo "setting the font" && setfont /usr/share/consolefonts/Lat15-VGA8.psf.gz && echo "successfull"

echo "mounting bell" && sh /home/chime/Linux-Scripts/Chime/mount-bell.sh && echo "successfull"

echo "opening terminals 2 thru 24" && echo "root-beer" | sudo -S sh /home/chime/Linux-Scripts/Chime/tty-script.sh && echo "successfull"
