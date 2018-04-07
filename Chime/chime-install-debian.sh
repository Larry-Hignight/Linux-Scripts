echo "Add Chime to the sudoers file" && echo ""
su -m
usermod -aG sudo username chime
apt-get install sudo
exit  # Exit from su
echo "" && echo "Finished adding Chime to the sudoers file" && sleep 3

sh chime-install-ubuntu.sh
