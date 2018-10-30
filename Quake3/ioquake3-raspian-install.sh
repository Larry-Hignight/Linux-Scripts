# These instructions for install ioquake3 on a Raspberry Pi are culled from the following sites:
# https://www.raspbian.org/RaspbianQuake3
# https://github.com/raspberrypi/quake3
# https://www.technologist.site/2016/11/05/how-to-install-and-benchmark-quake-iii-on-the-raspberry-pi/3/?doing_wp_cron=1538596800.7339758872985839843750

# Make sure you're up-to-date:

sudo apt-get update
sudo apt-get dist-upgrade
sudo rpi-update 192            # What is this doing?  Is this out of date?
sudo shutdown -r now           # Is this really necessary?


# Install required packages:

sudo apt-get install git gcc build-essential libsdl1.2-dev

# Download the Quake 3 source code:

mkdir -p ~/Downloads/ioquake3-files
cd ~/Downloads/ioquake3-files
git clone https://github.com/raspberrypi/quake3.git
cd quake3

# Edit build.sh in quake3 directory:

#change line 8 to this:  ARM_LIBS=/opt/vc/lib
#change line 16 to this: INCLUDES="-I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads"
#comment out line 19:    #CROSS_COMPILE=bcm2708-

# Build the game:
./build.sh


# It takes appx 1 hour to compile on a Raspberry Pi


# Find copies of the following somewhere (other guides will show you) and place in build/release-linux-arm/baseq3:
# pak0.pk3, pak1.pk3, pak2.pk3, pak3.pk3, pak4.pk3, pak5.pk3, pak6.pk3, pak7.pk3, pak8.pk3 

# your permissions for directFB access:
# sudo usermod -a -G video [your_username]

# Log out, log back in. This will allow you to run game as non-root. Works with other directFB/SDL based stuff, too. 
# If you do not intend to keep Quake 3 source code, you may reorganize files as suggested by this thread on RasPi Forum.

# Run ioquake3.arm. Shoot things.
# Highly non-scientific testing (i.e. having played a normal-Debian build a few days ago) suggests framerate is quite dramatically improved. Go Raspbian!
