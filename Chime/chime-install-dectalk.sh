## Install the speakup software for the DEC Talk Express
sudo apt-get -y install speakup-tools
sudo modprobe speakup_dectlk
sudo mv /etc/modules /etc/modules.orig
sudo cp modules /etc/modules
