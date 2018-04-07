3echo "Setting the font to a higher resolution font (ie more lines per screen)"
sh chime-install-setfont.sh
echo "" && echo "Finished setting the font to a higher resolution font (ie more lines per screen)" && sleep 3

echo "Installing the speakup software for the DEC Talk Express"
echo ""
sh chime-install-dectalk.sh
echo "" && echo "Finished installing the speakup software for the DEC Talk Express" && sleep 3

echo "Installing the application software"
echo ""
sh chime-install-applications.sh
echo "" && echo "Finished installing the application software" && sleep 3

echo "Installing Docker"
echo ""
sh ../Docker/docker-ce-install-ubuntu.sh
sh ../Docker/docker-compose-install-ubuntu.sh
echo "" && echo "Finished installing Docker" && sleep 3

echo "Installing tcsh as the default shell"
echo ""
sh chime-install-tcsh.sh
echo "" && echo "Finished installing tcsh as the default shell" && sleep 3

echo "Mounting file server using sshfs"
echo ""
sh mount-bell.sh
echo "" && echo "Finished mounting file server using sshfs" && sleep 3

echo "Adding symbolic links to scripts"
echo ""
sh chime-install-scripts.sh
echo "" && echo "Finished adding symbolic links to scripts" && sleep 3

echo "Removing old packages"
echo ""
sudo apt autoremove
echo "" && echo "Finished removing old packages" && sleep 3

echo "" && echo "Finished Installation"
