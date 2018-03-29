apt-get install emacs25-nox

## Change the PS1 prompt
PS1="chime$ "

## Change the framebuffer resolution
emacs /etc/default/grub

# Change line to:
# GRUB_CMDLINE_LINUX_DEFAULT="splash vga=792"  or 795 or 799 for higher resolutions
update-grub
shutdown -r now


## Install the speakup software for the DEC Talk Express
apt-get -y install speakup-tools
modprobe speakup_dectlk
emacs /etc/modules
# add the following on a line: speakup_dectlk

apt-get -y install zip
apt-get -y install rar
apt-get -y install unrar
apt-get -y install ncftp
apt-get -y install tcsh
apt-get -y install yash
apt-get -y install emacs-nox
apt-get -y install git
apt-get -y install dos2unix
apt-get -y install lynx
apt-get -y install alpine
apt-get -y install trn4
apt-get -y install ncftp
apt-get -y install uudeview

apt-get -y install vorbis-tools
apt-get -y install mpv
apt-get -y install mplayer
apt-get -y install madplay
apt-get -y install mikmod
apt-get -y install mpg123
apt-get -y install mpg321
apt-get -y install vobcopy
apt-get -y install dosemu
apt-get -y install beep

apt-get -y install ffmpeg   # Already installed; Must to use it... =\

# apt-get install python3      # This was already installed on the system 
apt-get -y install python3-pip     
pip3 install --upgrade pip 
pip3 install --upgrade youtube-dl
pip3 install --upgrade httpie


## Install Alsa sound
apt-get install alsa alsa-tools
adduser chime audio               # Add yourself to the group audio:
init 6                            # Reboot the system
alsamixer


## Mount bell on chime using sshfs
apt-get install sshfs
# For some reason, I couldn't add chime to the fuse group (ie fuse didn't exist)
# gpasswd -a $USER fuse
mkdir -p /home/chime/bell-data
sshfs -o idmap=user chime@192.168.0.6:/mnt/data/home/chime /home/chime/bell-data


## These packages were missing in the Ubuntu repositories
# pdftotext not available
# youtube-viewer from Trizen

apt autoremove
