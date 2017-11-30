## Change the PS1 prompt
PS1="chime$ "

## Change the framebuffer resolution
sudo emacs /etc/default/grub

# Change line to:
# GRUB_CMDLINE_LINUX_DEFAULT="splash vga=792"  or 795 or 799 for higher resolutions
sudo update-grub
sudo shutdown -r now


## Install the speakup software for the DEC Talk Express
sudo apt-get -y install speakup-tools
sudo modprobe speakup_dectlk
sudo emacs /etc/modules
# add the following on a line: speakup_dectlk


sudo apt-get -y install zip
sudo apt-get -y install rar
sudo apt-get -y install unrar

sudo apt-get -y install tcsh
sudo apt-get -y install yash
sudo apt-get -y install emacs-nox
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

sudo apt-get -y install ffmpeg   # Already installed; Must sudo to use it... =\

# sudo apt-get install python3      # This was already installed on the system 
sudo apt-get -y install python3-pip     
pip3 install --upgrade pip 
pip3 install --upgrade youtube-dl
sudo pip3 install --upgrade httpie


## Install Alsa sound
sudo apt-get install alsa alsa-tools
sudo adduser chime audio               # Add yourself to the group audio:
sudo init 6                            # Reboot the system
alsamixer


## Mount bell on chime using sshfs
sudo apt-get install sshfs
# For some reason, I couldn't add chime to the fuse group (ie fuse didn't exist)
# sudo gpasswd -a $USER fuse
mkdir -p /home/chime/bell-data
sshfs -o idmap=user chime@192.168.0.6:/mnt/data/home/chime /home/chime/bell-data


## These packages were missing in the Ubuntu repositories
# pdftotext not available
# youtube-viewer from Trizen

sudo apt autoremove
