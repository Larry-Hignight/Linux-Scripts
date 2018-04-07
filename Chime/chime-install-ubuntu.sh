echo "Setting the font to a higher resolution font (ie more lines per screen)"
echo ""
source chime-install-setfont.sh

echo "Installing the speakup software for the DEC Talk Express"
echo ""
source chime-install-dectalk.sh

echo "Installing the application software"
echo ""
source chime-install-applications.sh

echo "Installing tcsh as the default shell"
echo ""
source chime-install-tcsh.sh

echo "Mounting file server using sshfs"
echo ""
source mount-bell.sh

echo "Removing old packages"
echo ""
sudo apt autoremove

echo "Finished"
