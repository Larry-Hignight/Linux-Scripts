import sys,csv,os,time,subprocess
from operator import itemgetter
from subprocess import call

filePath=None
wait=3

listFile = '/usr/local/bin/.list.csv'

def main_menu(): 
    os.system('clear')
    print "number 1 re-cord existing stream"
    print "number 2 Listen to existing stream"
    print "number 3 Add new stream"
    print "number 4 Remove stream"
    print "number 5 Edit existing stream"
    print "number 6 Quit"
    choices=("1","2","3","4","5","6")
    choice_made = None
    while (choice_made == None):
        choice_made = menu_choice(choices)

    if (choice_made=="1"):
        choice = choose_stream()
        if (choice!="-1"):
            print choice
            record_stream(choice[1], get_path(),choice[2])
    elif (choice_made=="2"):
        choice = choose_stream()
        if (choice!="-1"):
            play_stream(choice[1])
    elif (choice_made=="3"):
        add_stream()
    elif (choice_made=="4"):
        choice = choose_stream()
        if (choice!="-1"):
            remove_stream(choice)
    elif (choice_made=="5"):
        choice = choose_stream()
        if (choice!="-1"):
            edit_stream(choice)
    elif (choice_made=="6"):
        sys.exit(0)

def edit_stream(stream):
    os.system('clear')
    print "existing alias: " + stream[0]
    alias=raw_input("Alias for new stream: ")
    print "existing URL: " + stream[1]
    url=raw_input("URL for new stream: ")
    data=get_data()
    for sublist in data:
        if sublist[0] == stream[0]:
            with open(listFile, 'rb') as inp, open('new.csv', 'wb') as out:
                writer = csv.writer(out)
                for row in csv.reader(inp):
                    if row[0] == stream[0]:
                        if (alias != ""):
                            row[0] = alias
                        if (url != ""):
                            row[1] = url
                    # print row
                    writer.writerow(row)

            os.remove(listFile)
            os.rename('new.csv', listFile)
            break

    # print stream
    time.sleep(wait)

def remove_stream(stream):
    data=get_data()
    for sublist in data:
        if sublist[0] == stream[0]:
            with open(listFile, 'rb') as inp, open('new.csv', 'wb') as out:
                writer = csv.writer(out)
                for row in csv.reader(inp):
                    if row[0] != stream[0]:
                        writer.writerow(row)
            os.remove(listFile)
            os.rename('new.csv', listFile)
            break


def choose_stream():
    os.system('clear')
    choices=list()
    stream_list=get_data()
    exit_entry_number=0
    for idx, val in enumerate(stream_list):
        print "number {0} {1}".format(idx+1 ,val[0])
        exit_entry_number=idx+1
        choices.append(str(idx+1))
    choice_made=None
    choices.append(str(exit_entry_number+1))
    print "number {0} Back".format(exit_entry_number+1)
    while (choice_made==None):
        choice_made=menu_choice(choices)
    if (len(stream_list) < int(choice_made)):
        return "-1"
    else :
        return stream_list[int(choice_made)-1]

def add_stream():
    #print "Add new stream not working yet"
    alias=raw_input("Alias for new stream: ")
    if (not check_alias(alias)):
        # print "don't have that one"
        stream=raw_input("URL for stream: ")
        extension=get_stream_type(stream)
        if extension==None:
            print "Problem with that url"
        else:
            new_fields=[alias,stream,extension]
            add_data(new_fields)
            print "New stream added"
        time.sleep(wait)
    else:
        print "This alias already taken"
        time.sleep(wait)

def get_stream_type(stream):
    print "Checking URL"
    #command = ['ffprobe', '-show_format', '-pretty', '-loglevel', 'quiet', stream]
    command = ['ffprobe', '-show_format', '-pretty', '-loglevel', 'quiet', '-protocol_whitelist', 'file,http,https,tcp,tls', stream]
    p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err =  p.communicate()
    for line in out.splitlines():
        if "mp3" in line:
            return "mp3"
        elif "hls" in line:
            return "mp4"

def play_stream(stream):
    print "number 1 On Board"
    print "number 2 Ensoniq"
    print "number 3 DGX"
    print "number 4 Audigy"
    card=raw_input("Which sound card. Default is On Board: ")
    if (card=="" or card=="1"):
         call(["mpv","--really-quiet","--no-video","--audio-device=alsa/plughw:CARD=Intel,DEV=0",str(stream)])
    elif (card=="2"):
         call(["mpv","--really-quiet","--no-video","--audio-device=alsa/plughw:CARD=AudioPCI,DEV=0",str(stream)])
    elif (card=="4"):
         call(["mpv","--really-quiet","--no-video","--audio-device=alsa/plughw:CARD=Audigy2,DEV=0",str(stream)])
    elif (card=="3"):
         call(["mpv","--really-quiet","--no-video","--audio-device=alsa/plughw:CARD=DGX,DEV=0", str(stream)])

def get_path():
    print "Input path and filename"
    return raw_input("Path:")

def record_stream(stream, filePath, extension):
    if (extension =="mp4"):
        print "mp4"
#        call(["sudo","ffmpeg","-re","-loglevel","error","-i",str(stream),"-c","copy","-bsf:a","aac_adtstoasc",str(filePath+"."+extension)])
#        call(["sudo","ffmpeg","-re","-loglevel","error","-i",str(stream),"-vn","-c:a","copy",str(filePath+"."+extension)])
#        call(["sudo","ffmpeg","-re","-loglevel","error","-i",str(stream),"-vn","-c:a","copy",str(filePath+"."+extension)])
#        call(["sudo","ffmpeg","-re","-i",str(stream),"-vn","-c:a","copy",str(filePath+"."+extension)])
        call(["ffmpeg","-re","-i",str(stream),"-vn","-c:a","copy",str(filePath+"."+extension)])
        call(["mv",str(filePath+"."+extension),str(filePath+".aac")])
    elif (extension =="mp3"):
        print "mp3"
#        call(["sudo","ffmpeg","-re","-loglevel","error","-i",str(stream),"-c","copy",str(filePath+"."+extension)])
        call(["ffmpeg","-re","-loglevel","error","-i",str(stream),"-c","copy",str(filePath+"."+extension)])
    # if (stream[1]=="-1"):
    #     print "no flag"
    #     call(["sudo","youtube-dl", "-o",filePath,stream[2]])
    # else:
    #     call(["sudo","youtube-dl","-o",filePath,"-f",stream[1],stream[2]])


def menu_choice(choices): 
    choice_made=raw_input("Choice: ")
    if choice_made in choices:
        # print "worked and stuff"
        return choice_made
    else:
        print "Invalid choice. Please choose again"
        return None

def main_menu_choice():
    choice_made=raw_input("choice:")
    if (choice_made=="1" or choice_made == "2" or choice_made == "3"):
        return choice_made
    else:
        print "Invalid choice. Please choose again:"
        return None

def get_data():
    with open(listFile, 'rb') as csvfile:
        data = list(csv.reader(csvfile))
    data = sorted(data, key=itemgetter(0))
    return data

def add_data(stream_data):
    with open(listFile, 'a') as f:
        writer = csv.writer(f)
        writer.writerow(stream_data)


def check_alias(alias):
    choice_made = False
    data = get_data()
    for lists in data:
        if(alias == lists[0]):
            choice_made = True
    return choice_made


# data = get_data()


if len(sys.argv) == 1:
    # print "menu"
    while (1==1):
        main_menu()
        # os.system('clear')
elif len(sys.argv) == 2:
    # print "one argument"
    data = get_data()
    if check_alias(sys.argv[1]):
        for lst in data:
            if (sys.argv[1] == lst[0]):
                play_stream(lst[1])
    else:
        print "Could not find that alias"
else:
    data = get_data()
    for lst in data:
        # print "sys.argv[1]:" + sys.argv[1]
        # print lst[0]
        if (sys.argv[1] == lst[0]):
            #call(["ffmpeg","-re","-i",str(lst[1]),"-c","copy","-bsf:a","aac_adtstoasc",str(sys.argv[2])])
            record_stream(str(lst[1]),str(sys.argv[2]),lst[2])
            # print lst[0]
            # print lst[1]
            # print lst[2]
            # if (lst[1]=="-1"):
            #     # print "no flag"
            #     call(["sudo","youtube-dl", "-o",sys.argv[2],lst[2]])
            # else:
            #     call(["sudo","youtube-dl","-o",sys.argv[2],"-f",lst[1],lst[2]])
        # else:
            # print "Could not find that alias"
            # sys.exit(0)

#sudo ffmpeg -re -i <url> -c copy -bsf:a aac_adtstoasc recording.mp4


