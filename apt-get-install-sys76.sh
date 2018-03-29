#wget http://download1.rstudio.org/rstudio-0.99.442-i386.deb
#sudo dpkg -i rstudio-0.99.442-i386.deb
#rm rstudio-0.99.442-i386.deb

#sudo apt-get -y install chromium-browser

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

