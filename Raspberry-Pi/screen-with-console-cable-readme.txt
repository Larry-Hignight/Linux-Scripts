Orient the Raspberry Pi such that the sdcard faces to the left.
Insert the serial cable pins into the topmost row of gpio pins:
Red, skip, black, white, green

Note - Only insert the red pin when powering the Pi over USB.

To connect to the Pi issue the following command:
sudo screen /dev/ttyUSB0 115200

Hit enter at the prompt to login.
