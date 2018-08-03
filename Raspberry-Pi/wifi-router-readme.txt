# Based on the instructions at https://thepi.io/how-to-use-your-raspberry-pi-as-a-wireless-access-point/

# Install hostapd and dnsmasq
#   hostapd is the package that lets us create a wireless hotspot
#   dnsmasq is an easy-to-use DHCP and DNS server

sudo apt-get install -y hostapd dnsmasq


# Turn off both services

sudo systemctl stop hostapd
sudo systemctl stop dnsmasq


# Configure a static IP for the wlan0 interface

sudo nano /etc/dhcpcd.conf


# Add the following lines at the end:
interface wlan0
static ip_address=192.168.0.10/24
denyinterfaces eth0
denyinterfaces wlan0


# Configure the DHCP server (dnsmasq)

sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig
sudo nano /etc/dnsmasq.conf

# Add these lines into your new configuration file:

interface=wlan0
  dhcp-range=192.168.0.11,192.168.0.30,255.255.255.0,24h


# Configure the access point host software (hostapd)

sudo nano /etc/hostapd/hostapd.conf

This should be a new file. Add the lines below to the file;  Make sure to replace NETWORK and PASSWORD with the correct settings.
interface=wlan0
bridge=br0
hw_mode=g
channel=7
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
ssid=NETWORK
wpa_passphrase=PASSWORD


rbsKQGKoD9lLM4nOkTWcbbLq72y3bs
