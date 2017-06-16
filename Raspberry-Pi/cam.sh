# The following command can be used to check if the camera is connected
# vcgencmd get_camera
# root@aplus ~ # vcgencmd get_camera   # Sample output:
# supported=1 detected=1

echo 'SLEEP'
sleep 900

echo "STARTING TIMELAPSE"

# Standard Pi Camera
#raspistill -w 1600 -h 1080 -vf -hf -tl 1000 -t 5000 -ts -o /home/pi/timelapse-%10d.jpg  # 5 SEC TEST

raspistill -w 1600 -h 1080 -vf -hf -tl 1000 -t 99999999 -ts -o /home/pi/timelapse-%10d.jpg  # RUN UNTIL THE MEMORY IS EXHAUSTED


# Pi Noir Camera
#raspistill -vf -hf -drc high -ex night -t 9000000 -tl 3000 -ts -o timelapse-%10d.jpg


echo "FINISHED TIMELAPSE"
#echo "FOR RC.LOCAL"
#exit 0
