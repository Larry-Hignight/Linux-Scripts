## Chromium OS Build
## http://www.chromium.org/chromium-os/developer-guide#TOC-Decide-where-your-source-will-live
sudo apt-get install git-core gitk git-gui subversion curl
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH="$PATH":`pwd`/depot_tools  ##You may want to add this to your .bashrc file

cd /tmp
cat > ./sudo_editor <<EOF
#!/bin/sh
echo Defaults \!tty_tickets > \$1          # Entering your password in one shell affects all shells
echo Defaults timestamp_timeout=180 >> \$1 # Time between re-requesting your password, in minutes
EOF
chmod +x ./sudo_editor
sudo EDITOR=./sudo_editor visudo -f /etc/sudoers.d/relax_requirements

git config --global user.email "larry.hignight@gamblitgaming.com"
git config --global user.name "Larry Hignight"

# Downloads repo
cd ${HOME}/chromiumos
# Optional: Add the "-g minilayout" option to do a minilayout checkout.
repo init -u https://chromium.googlesource.com/chromiumos/manifest.git 
#--repo-url https://chromium.googlesource.com/external/repo.git [-g minilayout]  ## I removed this from above
repo sync

# Setup the chroot environment
cros_sdk

# Look in ~/trunk/src/overlays for a complete list of boards
export BOARD=x86-generic

# Make sure that you in chroot
cd ~/trunk/src/scripts
./setup_board --board=${BOARD}

# Prompts for the cli 'chronos' password in chromiumos
./set_shared_user_password.sh

# The build packages will take over 90 minutes to finish
./build_packages --board=${BOARD}

# Builds a 'base developer image'
./build_image --board=${BOARD} --noenable_rootfs_verification dev

# The image produced by build_image will be located in ~/trunk/src/build/images/${BOARD}/versionNum/ 
# The most recent image produced for a given board will be symlinked to ~/trunk/src/build/images/${BOARD}/latest.
# IMPORTANT NOTE: It's up to you to delete old builds that you don't need. 
# Every time you run build_image, the command creates files that take up over 4GB of space(!).

# Look at your disk image (optional)
# The preferred way to mount the image you just built to look at it's contents is:
./mount_gpt_image.sh --board=${BOARD} -f $(./get_latest_image.sh --board=${BOARD})

# Again, don't forget to unmount the root filesystem when you're done:
./mount_gpt_image.sh --board=${BOARD} -u 

# Optionally, you can unpack the partition as separate files and mount them directly:
# cd ~/trunk/src/build/images/${BOARD}/latest
# ./unpack_partitions.sh chromiumos_image.bin
# mkdir -p rootfs
# sudo mount -o loop,ro part_3 rootfs

# This will do a loopback mount of the rootfs from your image to the location 
# ~/trunk/src/build/images/${BOARD}/latest/rootfs in your chroot.

# If you built with "--noenable_rootfs_verification" you can omit the "ro" option to mount to mount it read write.
# If you built an x86 Chromium OS image, you can probably even try chrooting into the image:
# sudo chroot ~/trunk/src/build/images/${BOARD}/latest/rootfs
# This is a little hacky (the Chromium OS rootfs isn't really designed to be a chroot for your host machine), 
# but it seems to work pretty well. Don't forget to exit this chroot when you're done.

# When you're done, unmount the root filesystem:
# sudo umount ~/trunk/src/build/images/${BOARD}/latest/rootfs 


# Put your image on a USB disk
# The easiest way to get your image running on your target computer is to put the image on a USB flash disk 
# (sometimes called a USB key), and boot the target computer from the flash disk. The first step is to insert 
# a USB flash disk (4GB or bigger) into your build computer. This disk will be completely erased, so make 
# sure it doesn't have anything important on it.  

# Wait ~10 seconds for the USB disk to register, then type the following command:
cd ../build/images
cros flash usb:// ${BOARD}/latest


# Boot from your USB disk
# You should be able to configure your target computer's BIOS to boot from your USB key. After you've done that, simply plug in your newly minted USB key and reboot your target computer – it should now boot from the Chromium OS image on your USB key. Your Chromium OS image may not have all the drivers to run all of the peripherals on your computer, but it should at least boot.
# 
# For specific information about what works on various different machines, see the Developer Hardware page.
# Installing your Chromium OS image to your hard disk
# Once you've booted from your USB key and gotten to the command prompt, you can install your Chromium OS image to the hard disk on your computer with this comman*sd:
# 
# /usr/sbin/chromeos-install


# Getting to a command prompt on Chromium OS
# Since you set the shared user password (with set_shared_user_password.sh) when you built your image, 
# you have the ability to login as the chronos user:
# 
# After your computer has booted to the Chromium OS login screen, press [ Ctrl ] [ Alt ] [ F2 ] to get a 
# text-based login prompt. ( [ F2 ] may appear as [ → ] on your Notebook keyboard.)
# 
# Login with the chronos user and enter the password you set earlier.
# 
# Because you built an image with developer tools, you also have an alternate way to get a terminal prompt. 
# The alternate#  shell is a little nicer (in the very least, it keeps your screen from dimming on you), even if it is a little harder t# o get to. To use this alternate shell:
# 
#     Go through the standard Chromium OS login screen (you'll need to setup a network, etc.) and get to the web browser.#   It's OK to login as guest.
#     Press [ Ctrl ] [ Alt ] [ T ] to get the crosh shell.
# Use the shell command to get the shell prompt. NOTE: you don't need to enter the chronos password here, 
# though you will still need the password if you want to use the sudo command.

