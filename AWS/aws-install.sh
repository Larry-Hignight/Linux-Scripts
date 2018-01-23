sudo apt update && sudo apt -y upgrade

echo "################################"
echo "## Setup a working Python Env ##"
echo "################################"

sudo apt -y install python3-pip
sudo apt -y install python3-venv
pip3 install --upgrade pip


echo "#######################"
echo "## Development Tools ##"
echo "#######################"
sudo apt -y install emacs24-nox


echo "##############################"
echo "## R & RStudio Installation ##"
echo "##############################"
sudo apt -y install r-base                # Note - I changed this from r-base-core; TODO: Review these packages
sudo apt -y install libjpeg62             # Required for RStudio
sudo apt -y install libssl-dev            # Required for the httr and openssl packages
sudo apt -y install libcurl4-openssl-dev  # Required for the RCurl package
sudo apt -y install libxml2-dev           # Required for the XML package


echo "#######################################################"
echo "## Hardware Info / OS Info                           ##"
echo "## http://www.binarytides.com/linux-cpu-information  ##"
echo "#######################################################"
sudo apt -y install conky
sudo apt -y install hardinfo
sudo apt -y install cpuid
sudo apt -y install inxi
sudo apt -y install htop
sudo apt -y install pmount  # A handy tool for mounting hardware (eg thumb drives)
sudo apt -y install wavemon # A cool ncurses based wifi monitor
#sudo apt -y install iotop  # Requires a sudoer to run it with root level access


echo "##################"
echo "## Web Browsers ##"
echo "##################"
sudo apt -y install lynx


# echo "######################"
# echo "## Java programming ##"
# echo "######################"
# sudo apt -y install openjdk-8-jdk
# sudo apt -y install ant
# sudo apt -y install maven2

