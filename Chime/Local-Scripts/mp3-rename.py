import os,subprocess,sys
sys.tracebacklimit = 0



def search_mp3s():
    files = [f for f in os.listdir(os.getcwd()) if os.path.isfile(f)]
    for f in files:
        artist=''
        title=''
        try:
            result=subprocess.check_output(['mp3info', f])
            for line in result.splitlines():
                if line.startswith("Artist") and ('.mp3' not in line):
                    artist=line.replace('Track:','').replace('Artist:','').strip().replace('\r', '').replace('\n', '').replace('(','').replace(')','')
                if line.startswith("Title") and ('.mp3' not in line):
                    title=line.replace('Track:','').replace('Title:','').strip().replace('\r', '').replace('\n', '').replace('(','').replace(')','')
        except subprocess.CalledProcessError as e:
            output_directory = sys.argv[1]
            if output_directory[-1:]=="/":
                output_directory=output_directory[:-1]
            subprocess.call(['mv',os.getcwd()+"/"+f,output_directory+"/"+f])
            # print os.getcwd()+"/"+f+" " + output_directory+"/"+f

        if title and artist:
            result=artist + "-" + title+".mp3"
            result=result.replace(' ','-')
            # print result
        old_file = os.path.join(os.getcwd(), f)
        new_file = os.path.join(os.getcwd(), result)
        # print new_file
        os.rename(old_file, new_file)



if (len(sys.argv)>1):
    if (os.path.isdir(sys.argv[1])):
        search_mp3s()
    else:
        print "directory added does not exist"

else:
    print "must include directory for untagged mp3s"