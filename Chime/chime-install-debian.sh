echo "Add Chime to the sudoers file"
echo ""
su -m
usermod -aG sudo username chime
apt-get install sudo
exit  # Exit from su

echo "Setting the font to a higher resolution font (ie more lines per screen)"
echo ""
sh chime-install-setfont.sh

echo "Installing the speakup software for the DEC Talk Express"
echo ""
sh chime-install-dectalk.sh

echo "Installing the application software"
echo ""
sh chime-install-applications.sh

echo "Installing Docker"
echo ""
sh ../Docker/docker-ce-debian-install.sh

echo "Installing tcsh as the default shell"
echo ""
sh chime-install-tcsh.sh

echo "Mounting file server using sshfs"
echo ""
sh mount-bell.sh

echo "Removing old packages"
echo ""
sudo apt autoremove

echo "Finished"
