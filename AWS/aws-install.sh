sudo apt-get update && sudo apt-get -y upgrade

echo "################################"
echo "## Setup a working Python Env ##"
echo "################################"

sudo apt-get -y install python3-pip
sudo apt-get -y install python3-venv
pip3 install --upgrade pip


echo "#######################"
echo "## Development Tools ##"
echo "#######################"
sudo apt-get -y install emacs24-nox


echo "##############################"
echo "## R & RStudio Installation ##"
echo "##############################"
sudo apt-get -y install r-base                # Note - I changed this from r-base-core; TODO: Review these packages
sudo apt-get -y install libjpeg62             # Required for RStudio
sudo apt-get -y install libssl-dev            # Required for the httr and openssl packages
sudo apt-get -y install libcurl4-openssl-dev  # Required for the RCurl package
sudo apt-get -y install libxml2-dev           # Required for the XML package


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


# echo "######################"
# echo "## Java programming ##"
# echo "######################"
# sudo apt-get -y install openjdk-8-jdk
# sudo apt-get -y install ant
# sudo apt-get -y install maven2

