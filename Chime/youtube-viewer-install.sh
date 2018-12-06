# Clone the project
https://github.com/trizen/youtube-viewer.git
cd youtube-viewer

# Build the project
cpan Module:Build
perl Build.PL
sudo ./Build installdeps
sudo ./Build install
cd ..
rm -rf youtube-viewer

# Example Usage:
# youtube-viewer --player=mpv --novideo -u=pbsnewshour
