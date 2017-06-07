# Based on the Docker CE for Ubuntu instruction at
# https://store.docker.com/editions/community/docker-ce-server-ubuntu
#
# Note:  This installation requires Ubuntu 14.04, 16.04 or 16.10.
# If you're unsure of the Ubuntu version, run 'lsb_release -cs'.
#
# The result should be one of the following:
#
#    Yakkety 16.10
#    Xenial 16.04
#    Trusty 14.04


sudo apt-get update && sudo apt-get upgrade

echo "=================================="
echo "== Setting up Docker Repository =="
echo "=================================="

sudo apt-get -y install apt-transport-https ca-certificates curl
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -

sudo add-apt-repository \
       "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
       $(lsb_release -cs) \
       stable"

sudo apt-get update


echo "=========================="
echo "== Installing Docker CE =="
echo "=========================="

sudo apt-get install docker-ce


echo "=========================================="
echo "== Test Docker Hello World Application  =="
echo "=========================================="

sudo docker run hello-world


echo "============================================"
echo "== Creating docker group for current user =="
echo "============================================"

#sudo groupadd docker
#sudo usermod -aG docker $USER

#docker run hello-world
