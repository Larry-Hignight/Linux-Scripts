echo "==== Upgrading the system ===="
sudo apt-get update && sudo apt-get -y upgrade

echo "==== Installing software for AWS ===="
aws-install.sh

echo "==== Running Github Setup Script ===="
../github-setup.sh

echo "==== Installing Docker and Docker Compose ===="
../Docker/docker-ce-install-ubuntu.sh
../Docker/docker-compose-install-ubuntu.sh
