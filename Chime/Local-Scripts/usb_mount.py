#!/usr/bin/env python
import sys, os
import subprocess

args = list(sys.argv)


if (len(args) > 2):
    print args
    print "/media/" + args[2]
    dir = "/media/" + args[2]
    dev = "/dev/" + args[2]
    if (args[1] == "add"):
        if os.path.isdir (dir):
            print "isdir"
            if os.path.ismount(dir):
                print ("Something is already mounted on" % dir)
            else:
                print "not ismount"
                subprocess.Popen(["mount",dev,dir])
        else:
            try:
                os.mkdir(dir)
            except OSError:
                print ("Creation of the directory %s failed" % dir)
            else:
                print ("Successfully created the directory %s " % dir)
                subprocess.Popen(["mount",dev,dir])
    elif (args[1] == "remove"):
        print "schtuff"
        if os.path.ismount(dir):
            print "removing it"
            subprocess.call(["umount",dir])
            os.rmdir(dir)
        else:
            print "this ain't no mount point"
    else:
	print "something went horribly wrong"
else:
    print "need more dammit"

