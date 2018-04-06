## Add Chime to the sudoers file
su -m
usermod -aG sudo username chime
apt-get install sudo
exit

## Change the PS1 prompt
PS1="chime$ "

## Install the speakup software for the DEC Talk Express
sudo apt-get -y install speakup-tools
modprobe speakup_dectlk
emacs /etc/modules
# add the following on a line: speakup_dectlk

sudo apt-get -y install zip
sudo apt-get -y install rar
sudo apt-get -y install unrar
sudo apt-get -y install ncftp
sudo apt-get -y install tcsh
sudo apt-get -y install yash
sudo apt-get -y install git
sudo apt-get -y install dos2unix
sudo apt-get -y install lynx
sudo apt-get -y install alpine
sudo apt-get -y install trn4
sudo apt-get -y install ncftp
sudo apt-get -y install uudeview

sudo apt-get -y install vorbis-tools
sudo apt-get -y install mpv
sudo apt-get -y install mplayer
sudo apt-get -y install madplay
sudo apt-get -y install mikmod
sudo apt-get -y install mpg123
sudo apt-get -y install mpg321
sudo apt-get -y install vobcopy
sudo apt-get -y install dosemu
sudo apt-get -y install beep

sudo apt-get -y install ffmpeg   # Already installed; Must to use it... =\

# sudo apt-get install python3      # This was already installed on the system
sudo apt-get -y install python3-pip
pip3 install --upgrade pip
pip3 install --upgrade youtube-dl
pip3 install --upgrade httpie


## Install Alsa sound
sudo apt-get install alsa alsa-tools
adduser chime audio               # Add yourself to the group audio:
init 6                            # Reboot the system
alsamixer


## Mount bell on chime using sshfs
sudo apt-get install -y sshfs
# For some reason, I couldn't add chime to the fuse group (ie fuse didn't exist)
# gpasswd -a $USER fuse

sudo apt autoremove
