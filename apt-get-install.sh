echo "############################"
echo "## Applications / General ##"
echo "############################"
sudo apt-get -y install anki
sudo apt-get -y install html2text
sudo apt-get -y install pdf2htmlex
sudo apt-get -y install steam
sudo apt-get -y install stellarium
sudo apt-get -y install xubuntu-community-wallpapers


echo "##################"
echo "## Web Browsers ##"
echo "##################"
sudo apt-get -y install chromium-browser
sudo apt-get -y install firefox
sudo apt-get -y install lynx


echo "#######################"
echo "## Multimedia - Main ##"
echo "#######################"
sudo apt-get -y install ffmpeg         # Audio and video encoder
sudo apt-get -y install vlc            # Media player
sudo apt-get -y install mpv            # CLI video player
sudo apt-get -y install openshot       # Non-Linear video editor
sudo apt-get -y install handbrake      # Compress DVD files
sudo apt-get -y install audacity       # Non-Linear sound editor
sudo apt-get -y install xloadimage     # xview - CLI image viewer
sudo apt-get -y install exif           # View EXIF data in JPEG files


echo "###########################"
echo "## Multimedia Additional ##"
echo "###########################"
sudo apt-get -y install mediainfo      # CLI for general media info (ie bitrate, etc)
sudo apt-get -y install mp3info        # CLI for MP3 tag info
sudo apt-get -y install vobcopy        # DVD copying
sudo apt-get -y install xmbc           # Media player
sudo apt-get -y install mplayer        # Media player
sudo apt-get -y install sound-juicer   # Music ripping from CDROM
sudo apt-get -y install cdparanoia     # Music ripping from CDROM


echo "###########################"
echo "## Multimedia Restricted ##"
echo "###########################"
sudo apt-get install ubuntu-restricted-extras
sudo /usr/share/doc/libdvdread4/install-css.sh

## These packages may not be necessary with the newer restricted-extras package
#sudo apt-get -y install w32codecs
#sudo apt-get -y install libdvdread4


echo "################################"
echo "## Setup a working Python Env ##"
echo "################################"

sudo apt-get -y install python3-pip
sudo apt-get -y install python3-venv
pip3 install --upgrade pip


echo "######################################"
echo "## Using PIP to install Python apps ##"
echo "######################################"

# Some apps may need to be install using sudo -H
pip install --upgrade youtube-dl   # Download streaming videos
pip install --upgrade podfox       # A command line podcatcher


echo "##############################################"
echo "## SSHFS - Mount remote directories via SSH ##"
echo "##  https://help.ubuntu.com/community/SSHFS ##"
echo "##############################################"

# sudo apt-get -y install sshfs
# sudo gpasswd -a $USER fuse
# mkdir ~/data
# sshfs -o idmap=user $USER@remote-host:/remote/data ~/data


echo "######################"
echo "## Database Clients ##"
echo "######################"
sudo apt-get -y install postgresql-client
sudo apt-get -y install mysql-workbench


echo "########################"
echo "## Design / CAD Tools ##"
echo "########################"
sudo apt-get -y install blender
sudo apt-get -y install openscad


echo "#######################"
echo "## Development Tools ##"
echo "#######################"
sudo apt-get -y install emacs25
sudo apt-get -y install git
sudo apt-get -y install dos2unix
sudo apt-get -y install meld
sudo apt-get -y install screen
sudo apt-get -y install xvfb


echo "##############################"
echo "## R & RStudio Installation ##"
echo "##############################"
sudo apt-get -y install r-base                # Note - I changed this from r-base-core; TODO: Review these packages
sudo apt-get -y install libjpeg62             # Required for RStudio
sudo apt-get -y install libssl-dev            # Required for the httr and openssl packages
sudo apt-get -y install libcurl4-openssl-dev  # Required for the RCurl package
sudo apt-get -y install libxml2-dev           # Required for the XML package

wget https://download1.rstudio.org/rstudio-xenial-1.1.423-amd64.deb
sudo dpkg -i rstudio-xenial-1.1.423-amd64.deb
rm rstudio-xenial-1.1.423-amd64.deb


echo "######################"
echo "## Java programming ##"
echo "######################"
sudo apt-get -y install openjdk-9-jdk
sudo apt-get -y install ant
sudo apt-get -y install maven2


echo "##################"
echo "## Netbeans IDE ##"
echo "##################"
## Note:  Be sure to download the latest version
## wget http://download.netbeans.org/netbeans/8.0/final/bundles/netbeans-8.0-javas#e-linux.sh
#chmod +x netbeans-8.0.2-javase-linux.sh
#./netbeans-8.0.2-javase-linux.sh


echo "####################"
echo "## Android Studio ##"
echo "####################"
# Install android-studio
# Manually download the latest version of AndroidStudio
# http://developer.android.com/sdk/index.html
# Simply unzip then run the setup script in bin
# Note:  There is mention of the KVM kernel option on Linux that should improve emulation performance


echo "######################################"
echo "## Additional Programming languages ##"
echo "######################################"
sudo apt-get -y install octave
sudo apt-get -y install clisp
sudo apt-get -y install swi-prolog
sudo apt-get -y install ucblogo
#sudo apt-get -y install clojure1.6
#sudo apt-get -y install leiningen
#sudo apt-get -y install scala
#sudo apt-get -y install ghc
#sudo apt-get -y install julia


# Instructions for installing Torch - http://torch.ch/docs/getting-started.html
#git clone https://github.com/torch/distro.git ~/torch --recursive
#cd ~/torch; bash install-deps;
#./install.sh


echo "#######################################################"
echo "## Hardware Info / OS Info                           ##"
echo "## http://www.binarytides.com/linux-cpu-information  ##"
echo "#######################################################"
sudo apt-get -y install conky
sudo apt-get -y install hardinfo
sudo apt-get -y install cpuid
sudo apt-get -y install inxi
sudo apt-get -y install htop
sudo apt-get -y install pmount       # A handy tool for mounting hardware (eg thumb drives)
sudo apt-get -y install wavemon      # A cool ncurses based wifi monitor
#sudo apt-get -y install iotop       # Requires a sudoer to run it with root level access
sudo apt-get -y install exfat-utils  # Support for the exfat filesystem


echo "################################"
echo "## Games - Benchmarks - Demos ##"
echo "################################"
sudo apt-get -y install gnubg            # GNU Backgammon
sudo apt-get -y install neverball        # Marble balance game
sudo apt-get -y install briquolo         # Breakout style game
sudo apt-get -y install criticalmass     # Galaxian style game
sudo apt-get -y install d1x-rebirth      # Port of Descent 1
sudo apt-get -y install d2x-rebirth      # Port of Descent 2
sudo apt-get -y install glhack           # OpenGL version of Nethack
sudo apt-get -y install gnubik           # 3D Rubik's cube game
sudo apt-get -y install gnujump          # Platform game
sudo apt-get -y install xracer
sudo apt-get -y install torcs
sudo apt-get -y install trackballs
sudo apt-get -y install freeciv
sudo apt-get -y install openuniverse

sudo apt-get -y install glmark2          # OpenGL benchmark - Pretty cool
sudo apt-get -y install glmark2-drm      # Core dumped on my laptop
sudo apt-get -y install glmark2-es2      # What is the es here?!
sudo apt-get -y install glmark2-es2-mir  # Didn't work on my laptop
sudo apt-get -y install glmark2-mir      # Didn't work on my laptop
sudo apt-get -y install glmark2-wayland  # Didn't work on my laptop
sudo apt-get -y install glmemperf        # Benchmark for OpenGL ES
sudo apt-get -y install globs            # GL Open Benchmark Suite
                                         # Does this install the previous bm's?

sudo apt-get -y install fraqtive         # Mandlebrot and Julia set viewer

#sudo apt-get -y install amoeba       # OpenGL video rendering (It's just OK)


#echo "##########################"
#echo "## Virtualization tools ##"
#echo "##########################"
#sudo apt-get -y install virtualbox-qt
#sudo apt-get -y install vagrant


## Chrome - not available via apt-get / Note - the 32 bit version is no longer being maintained
#wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb
#sudo dpkg -i google-chrome-stable_current_amd64.deb
#sudo apt-get -f install
#google-chrome http://www.yahoo.com


#echo "##################"
#echo "## Google Earth ##"
#echo "##################"
## Try the following instructions instead: https://help.ubuntu.com/community/GoogleEarth

#wget http://dl.google.com/dl/earth/client/current/google-earth-stable_current_amd64.deb #  64bit
#sudo apt-get -y install libc6-i386 libglib2.0-0:i386 libsm6:i386 libglu1-mesa:i386 libgl1-mesa-glx:i386 libxext6:i386 libxrender1:i386 libx11-6:i386 libfontconfig1:i386 lsb-core

##Still has an issue with ia32-libs not being installed, yet not installable using apt-get
# Double-click the downloaded .deb package to install it using the Ubuntu Software Center.
# After installation you should find Google Earth in the Applications -> Internet menu or through the Dash.

## Old method:  Not working...
#wget http://dl.google.com/dl/earth/client/current/google-earth-stable_current_amd64.deb #  64bit
#sudo apt-get -y install lib32z1 lib32ncurses5 lib32bz2-1.0
#sudo dpkg -i google-earth-stable*.deb
#sudo apt-get -f install

# Networking / Security
#sudo apt-get -y install aircrack-ng john #kismet  ##kismet requires setuid root?!
#wget ftp://mirrors.kernel.org/openwall/wordlists/languages/English/3-large/lower.gz
#wget ftp://mirrors.kernel.org/openwall/wordlists/languages/Spanish/lower.gz
#wget ftp://mirrors.kernel.org/openwall/wordlists/languages/French/lower.gz
#wget ftp://mirrors.kernel.org/openwall/wordlists/languages/French/mixed.gz
