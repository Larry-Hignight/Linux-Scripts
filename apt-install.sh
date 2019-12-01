echo "############################"
echo "## Applications / General ##"
echo "############################"
sudo apt -y install anki
sudo apt -y install html2text
sudo apt -y install pdf2htmlex
sudo apt -y install steam
sudo apt -y install stellarium
sudo apt -y install xubuntu-community-wallpapers


echo "##################"
echo "## Web Browsers ##"
echo "##################"
sudo apt -y install chromium-browser
sudo apt -y install firefox
sudo apt -y install lynx


echo "#######################"
echo "## Multimedia - Main ##"
echo "#######################"
sudo apt -y install ffmpeg         # Audio and video encoder
sudo apt -y install vlc            # Media player
sudo apt -y install mpv            # CLI video player
sudo apt -y install openshot       # Non-Linear video editor
sudo apt -y install handbrake      # Compress DVD files
sudo apt -y install audacity       # Non-Linear sound editor
sudo apt -y install xloadimage     # xview - CLI image viewer
sudo apt -y install exif           # View EXIF data in JPEG files


echo "###########################"
echo "## Multimedia Additional ##"
echo "###########################"
sudo apt -y install mediainfo      # CLI for general media info (ie bitrate, etc)
sudo apt -y install mp3info        # CLI for MP3 tag info
sudo apt -y install vobcopy        # DVD copying
sudo apt -y install xmbc           # Media player
sudo apt -y install mplayer        # Media player
sudo apt -y install sound-juicer   # Music ripping from CDROM
sudo apt -y install cdparanoia     # Music ripping from CDROM


echo "#######################"
echo "## DVD CSS Libraries ##"
echo "#######################"
sudo apt-get install libdvd-pkg
sudo dpkg-reconfigure libdvd-pkg


echo "####################"
echo "## Bluray Support ##"
echo "####################"
sudo apt install libbluray2
sudo apt install libbluray-bdj libbluray-bin libbluray-dev libbluray-doc
sudo apt install libaacs0
mkdir ~/.config/aacs && cd ~/.config/aacs
wget http://vlc-bluray.whoknowsmy.name/files/KEYDB.cfg


echo "################################"
echo "## Setup a working Python Env ##"
echo "################################"

sudo apt -y install python3-pip
sudo apt -y install python3-venv
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

# sudo apt -y install sshfs
# sudo gpasswd -a $USER fuse
# mkdir ~/data
# sshfs -o idmap=user $USER@remote-host:/remote/data ~/data


echo "######################"
echo "## Database Clients ##"
echo "######################"
sudo apt -y install postgresql-client
sudo apt -y install mysql-workbench


echo "########################"
echo "## Design / CAD Tools ##"
echo "########################"
sudo apt -y install blender
sudo apt -y install openscad


echo "#######################"
echo "## Development Tools ##"
echo "#######################"
sudo apt -y install emacs25
sudo apt -y install git
sudo apt -y install devscripts
sudo apt -y install ubuntu-dev-tools
sudo apt -y install meld
sudo apt -y install dos2unix
sudo apt -y install screen
sudo apt -y install xvfb


echo "##############################"
echo "## R & RStudio Installation ##"
echo "##############################"
sudo apt -y install r-base                     # Note - I changed this from r-base-core; TODO: Review these packages
sudo apt -y install libjpeg62                  # Required for RStudio
sudo apt -y install libssl-dev                 # Required for the httr and openssl packages
sudo apt -y install libcurl4-openssl-dev       # Required for the RCurl package
sudo apt -y install libxml2-dev                # Required for the XML package
sudo apt -y install libcairo2 libcairo2-dev    # Required for the Cairo and imager packages

wget https://download1.rstudio.org/rstudio-xenial-1.1.423-amd64.deb
sudo dpkg -i rstudio-xenial-1.1.423-amd64.deb
rm rstudio-xenial-1.1.423-amd64.deb


echo "######################"
echo "## Java programming ##"
echo "######################"
sudo apt -y install openjdk-9-jdk
sudo apt -y install ant
sudo apt -y install maven2


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
sudo apt -y install octave
sudo apt -y install clisp
sudo apt -y install swi-prolog
sudo apt -y install ucblogo
#sudo apt -y install clojure1.6
#sudo apt -y install leiningen
#sudo apt -y install scala
#sudo apt -y install ghc
#sudo apt -y install julia


# Instructions for installing Torch - http://torch.ch/docs/getting-started.html
#git clone https://github.com/torch/distro.git ~/torch --recursive
#cd ~/torch; bash install-deps;
#./install.sh


echo "#######################################################"
echo "## Hardware Info / OS Info                           ##"
echo "## http://www.binarytides.com/linux-cpu-information  ##"
echo "#######################################################"
sudo apt -y install conky
sudo apt -y install hardinfo
sudo apt -y install cpuid
sudo apt -y install inxi
sudo apt -y install htop
sudo apt -y install macchanger
sudo apt -y install pmount             # A handy tool for mounting hardware (eg thumb drives)
sudo apt -y install wavemon            # A cool ncurses based wifi monitor
# sudo apt -y install iotop            # Requires a sudoer to run it with root level access
sudo apt -y install exfat-utils        # Support for the exfat filesystem
# sudo apt -y install intel-gpu-tools  # A top like program for Intel GPUs (sudo intel_gpu_top)


echo "################################"
echo "## Games - Benchmarks - Demos ##"
echo "################################"
sudo apt -y install xboard           # X Window System Chess Board
sudo apt -y install gnuchess         # GNU Chess engine
sudo apt -y install crafty           # Crafty chess engine
sudo apt -y install stockfish        # Stockfish chess engine
sudo apt -y install fruit            # Fruit 2.1 chess engine
sudo apt -y install toga2            # Toga II chess engine
sudo apt -y install scid             # Chess database with play and training functionality
sudo apt -y install polyglot         # Chess engine protocol adaptor, connects UCI engines to xboard
sudo apt -y install quarry           # Board games Go, Amazons, and Reversi (a.k.a. Othello)
sudo apt -y install gnubg            # GNU Backgammon
sudo apt -y install neverball        # Marble balance game
sudo apt -y install briquolo         # Breakout style game
sudo apt -y install criticalmass     # Galaxian style game
sudo apt -y install d1x-rebirth      # Port of Descent 1
sudo apt -y install d2x-rebirth      # Port of Descent 2
sudo apt -y install glhack           # OpenGL version of Nethack
sudo apt -y install gnubik           # 3D Rubik's cube game
sudo apt -y install gnujump          # Platform game
sudo apt -y install xracer
sudo apt -y install torcs
sudo apt -y install trackballs
sudo apt -y install freeciv
sudo apt -y install openuniverse

sudo apt -y install glmark2          # OpenGL benchmark - Pretty cool
sudo apt -y install glmark2-drm      # Core dumped on my laptop
sudo apt -y install glmark2-es2      # What is the es here?!
sudo apt -y install glmark2-es2-mir  # Didn't work on my laptop
sudo apt -y install glmark2-mir      # Didn't work on my laptop
sudo apt -y install glmark2-wayland  # Didn't work on my laptop
sudo apt -y install glmemperf        # Benchmark for OpenGL ES
sudo apt -y install globs            # GL Open Benchmark Suite
                                         # Does this install the previous bm's?

sudo apt -y install fraqtive         # Mandlebrot and Julia set viewer

#sudo apt -y install amoeba       # OpenGL video rendering (It's just OK)


#echo "##########################"
#echo "## Virtualization tools ##"
#echo "##########################"
#sudo apt -y install virtualbox-qt
#sudo apt -y install vagrant


## Chrome - not available via apt / Note - the 32 bit version is no longer being maintained
#wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb
#sudo dpkg -i google-chrome-stable_current_amd64.deb
#sudo apt -f install
#google-chrome http://www.yahoo.com
