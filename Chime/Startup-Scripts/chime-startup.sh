echo "running the inflection script" && echo "root-beer" | sudo -S sh /home/chime/Linux-Scripts/Chime/inflection.sh && echo "successful"

echo "setting the font" && setfont /usr/share/consolefonts/Lat15-VGA8.psf.gz && echo "successfull"

echo "mounting bell" && sudo mount -a && echo "mount successful"

sudo cp /home/chime/linux-scripts/characters /sys/accessibility/speakup/i18n/
