import sys,os, subprocess
from subprocess import call


print type(len(sys.argv))
if len(sys.argv) < 2:
  print "Usage: Must specify a file for folder"
else:
  args = list(sys.argv)
  args.pop(0)
  for a in args:
    call(["sudo","ffmpeg","-re","-loglevel","error","-i",a,"-q:a","0","-map","a",str(os.path.splitext(a)[0]+".mp3")])
    # -q:a 0 -map a k.mp3
    # print os.path.splitext(a)[0]
