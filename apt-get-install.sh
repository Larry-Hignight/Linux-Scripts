echo "############################"
echo "## Applications / General ##"
echo "############################"
sudo apt-get -y install steam
sudo apt-get -y install stellarium
sudo apt-get -y install xubuntu-community-wallpapers


echo "########################"
echo "## Multimedia Related ##"
echo "########################"
sudo apt-get install ubuntu-restricted-extras
sudo /usr/share/doc/libdvdread4/install-css.sh

## These packages may not be necessary with the newer restricted-extras package
#sudo apt-get -y install w32codecs
#sudo apt-get -y install libdvdread4

sudo apt-get -y install mediainfo
sudo apt-get -y install ffmpeg
sudo apt-get -y install vobcopy
sudo apt-get -y install vlc
sudo apt-get -y install xmbc
sudo apt-get -y install openshot
sudo apt-get -y install handbrake
sudo apt-get -y install mplayer
sudo apt-get -y install audacity
sudo apt-get -y install sound-juicer
sudo apt-get -y install cdparanoia
sudo apt-get -y install xloadimage     # xview - cli image viewer

# Install youtube-dl using pip
sudo apt-get -y install python-pip
pip install --upgrade pip
pip install --upgrade youtube-dl


echo "########################"
echo "## Design / CAD Tools ##"
echo "########################"
sudo apt-get -y install blender
sudo apt-get -y install openscad


echo "#######################"
echo "## Development Tools ##"
echo "#######################"
sudo apt-get -y install emacs24
sudo apt-get -y install git
sudo apt-get -y install dos2unix
sudo apt-get -y install meld
sudo apt-get -y install screen


echo "######################"
echo "## Java programming ##"
echo "######################"
sudo apt-get -y install openjdk-8-jdk
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


echo "##############################"
echo "## R & RStudio Installation ##"
echo "##############################"
sudo apt-get -y install r-base             # Note - I changed this from r-base-core
sudo apt-get -y install libjpeg62          # Required for RStudio
sudo apt-get install libcurl4-openssl-dev  # Required for the RCurl package
sudo apt-get install libxml2-dev           # Required for the XML package
#wget http://download1.rstudio.org/rstudio-0.99.442-i386.deb
#sudo dpkg -i rstudio-0.99.442-i386.deb
#rm rstudio-0.99.442-i386.deb


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
sudo apt-get -y install pmount  # A handy tool for mounting hardware (eg thumb drives)
sudo apt-get -y install wavemon # A cool ncurses based wifi monitor
#sudo apt-get -y install iotop  # Requires a sudoer to run it with root level access


echo "##################"
echo "## Web Browsers ##"
echo "##################"
sudo apt-get -y install lynx
sudo apt-get -y install chromium-browser


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


## Networking / Security
#sudo apt-get -y install aircrack-ng john #kismet  ##kismet requires setuid root?!
#wget ftp://mirrors.kernel.org/openwall/wordlists/languages/English/3-large/lower.gz
#wget ftp://mirrors.kernel.org/openwall/wordlists/languages/Spanish/lower.gz
#wget ftp://mirrors.kernel.org/openwall/wordlists/languages/French/lower.gz
#wget ftp://mirrors.kernel.org/openwall/wordlists/languages/French/mixed.gz


##########################################################
## THIS ENTIRE SECTION CAN/SHOULD BE REPLACED BY DOCKER ##
##########################################################

#echo "##############"
#echo "## Services ##"
#echo "##############"
#sudo apt-get -y install tomcat7
#sudo apt-get -y install zookeeper
#wget http://mirrors.advancedhosters.com/apache/activemq/5.10.0/apache-activemq-5.10.0-bin.tar.gz

#echo "##########################"
#echo "## Virtualization tools ##"
#echo "##########################"
#sudo apt-get -y install virtualbox-qt
#sudo apt-get -y install vagrant

## Cassandra - http://docs.datastax.com/en/cassandra/2.2/cassandra/install/installDeb.html
#echo "deb http://debian.datastax.com/community stable main" | sudo tee -a /etc/apt/sources.list.d/cassandra.sources.list
#curl -L http://debian.datastax.com/debian/repo_key | sudo apt-key add -
#sudo apt-get update
#sudo apt-get install dsc22
#sudo apt-get install cassandra-tools
