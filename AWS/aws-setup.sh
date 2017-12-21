echo "==== Upgrading the system ===="
sudo apt-get update && sudo apt-get -y upgrade
echo "" && echo "" && echo "" && sleep 3

echo "==== Installing software for AWS ===="
sh aws-install.sh
echo "" && echo "" && echo "" && sleep 3

echo "==== Running Github Setup Script ===="
sh ../github-setup.sh
echo "" && echo "" && echo "" && sleep 3

echo "==== Installing Docker and Docker Compose ===="
../Docker/docker-ce-install-ubuntu.sh
../Docker/docker-compose-install-ubuntu.sh
echo "" && echo ""
