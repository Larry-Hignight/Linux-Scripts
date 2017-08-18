# Based on the following instructions: https://docs.docker.com/compose/install/#install-compose

# Specify the version of Docker Compose that you want to install
VERSION="1.15.0"
echo "Installing Docker Compose version $VERSION"

# Download the release
sudo curl -L https://github.com/docker/compose/releases/download/$VERSION/docker-compose-`uname -s`-`uname -m` \
     -o /usr/local/bin/docker-compose

# Apply executable permissions to the binary
sudo chmod +x /usr/local/bin/docker-compose

# Test the installation.
docker-compose --version
