echo "Add Chime to the sudoers file" && echo ""
su -m
usermod -aG sudo username chime
apt-get install sudo
exit  # Exit from su
echo "" && echo "Finished adding Chime to the sudoers file" && sleep 3

echo "Setting the font to a higher resolution font (ie more lines per screen)" && echo ""
sh chime-install-setfont.sh
echo "" && echo "Finished setting the font to a higher resolution font (ie more lines per screen)" && sleep 3

echo "Installing the speakup software for the DEC Talk Express" && echo ""
sh chime-install-dectalk.sh
echo "" && echo "Finished installing the speakup software for the DEC Talk Express" && sleep 3

echo "Installing the application software" && echo ""
sh chime-install-applications.sh
echo "" && echo "Finished installing the application software" && sleep 3

echo "Installing Docker" && echo ""
sh ../Docker/docker-ce-debian-install.sh
echo "" && echo "Finished installing Docker" && sleep 3

echo "Installing tcsh as the default shell" && echo ""
sh chime-install-tcsh.sh
echo "" && echo "Finished installing tcsh as the default shell" && sleep 3

echo "Mounting file server using sshfs" && echo ""
sh mount-bell.sh
echo "" && echo "Finished mounting file server using sshfs" && sleep 3

echo "Removing old packages" && echo ""
sudo apt autoremove
echo "" && echo "Finished removing old packages" && sleep 3

echo "Finished"
