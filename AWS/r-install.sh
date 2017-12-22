echo "####################"
echo "## R & R Packages ##"
echo "####################"
echo ""

sudo apt-get -y install r-base                # Note - I changed this from r-base-core; TODO: Review these packages
sudo apt-get -y install libjpeg62             # Required for RStudio
sudo apt-get -y install libssl-dev            # Required for the httr and openssl packages
sudo apt-get -y install libcurl4-openssl-dev  # Required for the RCurl package
sudo apt-get -y install libxml2-dev           # Required for the XML package

echo "" && echo "Installing Image and Networking Packages" && sleep 3
R -e "install.packages(c('jpeg', 'png'), repos='https://cran.rstudio.com/', lib='~/R')"
R -e "install.packages(c('curl', 'jsonlite', 'openssl', 'uuid'), repos='https://cran.rstudio.com/', lib='~/R')"

echo "" && echo "Installing Hadley's Packages" && sleep 3
R -e "install.packages(c('data.table', 'httr', 'lubridate', 'reshape2', 'stringr'), repos='https://cran.rstudio.com/', lib='~/R')"
R -e "install.packages(c('plyr', 'dplyr'), repos='https://cran.rstudio.com/', lib='~/R')"

echo "" && echo "Installing Visualization Packages" && sleep 3
R -e "install.packages('googleVis', repos='https://cran.rstudio.com/', lib='~/R')"
R -e "install.packages('ggplot2', repos='https://cran.rstudio.com/', lib='~/R')"
R -e "install.packages(c('maps', 'mapproj'), repos='https://cran.rstudio.com/', lib='~/R')"

echo "" && echo "Installing Shiny and RMarkdown" && sleep 3
R -e "install.packages('rmarkdown', repos='https://cran.rstudio.com/', lib='~/R')"
R -e "install.packages('shiny', repos='https://cran.rstudio.com/', lib='~/R')"
