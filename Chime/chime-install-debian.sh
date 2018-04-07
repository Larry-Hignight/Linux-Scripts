echo "Add Chime to the sudoers file" && echo ""
su -m
usermod -aG sudo username chime
apt-get install sudo
exit  # Exit from su
echo "" && echo "Finished adding Chime to the sudoers file" && sleep 3

## The Ubuntu and Debian installation scripts are nearly the same from this point forward
sh chime-install-ubuntu.sh

## Install the 'esr' version of Firefox
sudo apt-get install -y firefox-esr
