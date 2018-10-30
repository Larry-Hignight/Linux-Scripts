# These instructions for install ioquake3 are based on the following sites:
# http://ioquake3.org/
# https://jonathan.bergknoff.com/journal/installing-quake3-linux-mint
# https://help.ubuntu.com/community/Games/Native/QuakeIIIArena

# Create an ioquake directory in Downloads w/ the necessary files

mkdir -p ~/Downloads/ioquake3-files
cd ~/Downloads/ioquake3-files

wget http://ioquake3.org/get-it/  # engine
wget http://ioquake3.org/get-it/  # data

wget the pak0 file from github
chmod 755 

# Run the engine install script
# Install the game in ~/ioquake3
# Skip creating a symlink to ioquake3 on the path
./ioquake3-1.36-7.1.x86_64.run

# Run the data install script
# Add the symlink to home then delete it later
./ioquake3-q3a-1.32-9.run
# rm the symlink

# Move the pak0.pk3 file to the baseq3 directory
mv pak0.pk3 ~/ioquake3/baseq3/


# TODO - It is recommended to update the ioquake3 installation with the nightly test build
