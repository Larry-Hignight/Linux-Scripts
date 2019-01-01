echo "running the inflection script" && inflection && echo "successful"

echo "setting tty1 to 135 lines" && stty rows 135 && echo "successful"

echo "mounting bell" && sudo mount -a && echo "successful"

echo "customizing typing speech in speakup" && sudo cp /home/chime/linux-scripts/characters /sys/accessibility/speakup/i18n/ && echo "successful"
