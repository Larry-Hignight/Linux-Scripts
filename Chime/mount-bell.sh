mkdir -p /home/chime/chime-backup
mkdir -p /home/chime/jfk
mkdir -p /home/chime/mp3
mkdir -p /home/chime/radio-streams
mkdir -p /home/chime/uncategorized
mkdir -p /home/chime/videos

sshfs -o idmap=user chime@192.168.0.5:/mnt/data/home/chime/chime-backup   /home/chime/chime-backup
sshfs -o idmap=user chime@192.168.0.5:/mnt/data/home/chime/jfk            /home/chime/jfk
sshfs -o idmap=user chime@192.168.0.5:/mnt/data/home/chime/mp3            /home/chime/mp3
sshfs -o idmap=user chime@192.168.0.5:/mnt/data/home/chime/radio-streams  /home/chime/radio-streams
sshfs -o idmap=user chime@192.168.0.5:/mnt/data/home/chime/uncategorized  /home/chime/uncategorized
sshfs -o idmap=user chime@192.168.0.5:/mnt/data/home/chime/videos         /home/chime/videos
