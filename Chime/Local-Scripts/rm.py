import sys,os
from subprocess import call

if len(sys.argv) < 2:
    print "Usage: Must specify a file for folder"
else:
    args = list(sys.argv)
    args.pop(0)
    for a in args:
        call(["mv",a,"/Trash"])
   