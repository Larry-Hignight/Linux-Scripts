echo "running the inflection script" && sudo sh /usr/local/bin/inflection.sh && echo "successful"

echo "setting tty1 to 135 lines" && stty rows 135 && echo "successful"

echo "mounting bell" && sudo mount -a && echo "successful"

echo "customizing typing speech in speakup" && sudo cp /usr/local/bin/characters /sys/accessibility/speakup/i18n/ && echo "successful"
