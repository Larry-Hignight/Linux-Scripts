echo "SLEEP"
sleep 120

echo "STARTING TIMELAPSE"
raspivid -vf -hf -t 180000 -o /home/pi/vid-1.mp4
raspivid -vf -hf -t 180000 -o /home/pi/vid-2.mp4
raspivid -vf -hf -t 180000 -o /home/pi/vid-3.mp4
raspivid -vf -hf -t 180000 -o /home/pi/vid-4.mp4
raspivid -vf -hf -t 180000 -o /home/pi/vid-5.mp4
raspivid -vf -hf -t 180000 -o /home/pi/vid-6.mp4
raspivid -vf -hf -t 180000 -o /home/pi/vid-7.mp4
raspivid -vf -hf -t 180000 -o /home/pi/vid-8.mp4
raspivid -vf -hf -t 180000 -o /home/pi/vid-9.mp4
raspivid -vf -hf -t 180000 -o /home/pi/vid-10.mp4

echo "FINISHED TIMELAPSE"
