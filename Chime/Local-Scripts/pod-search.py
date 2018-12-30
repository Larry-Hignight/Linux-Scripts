import urllib, json, sys

args = list(sys.argv)
args.pop(0)
# print args


if (len(args) > 0):
    insert = ""
    if args > 1:
        insert=insert+args.pop(0)
        for arg in args:
            insert=insert+"+"+arg
    else:
        insert=insert+arg.pop(0)

    url = "https://itunes.apple.com/search?term={0}&entity=podcast".format(insert)
    response = urllib.urlopen(url)
    data = json.loads(response.read())
    print "results", data['resultCount']
    for idx, val in enumerate(data['results']):
        print "{0}. {1} ({2})".format(idx+1, val['artistName'].encode('utf8'), val['collectionName'].encode('utf8'))


    possibles=[]
    if int(data['resultCount']) > 1:
        possibles=range(1, int(data['resultCount'])+1)
    else:
        possibles.append(1)

    result=raw_input("Choice: ")

    if ((result != "") and (int(result) in possibles)):
        print data['results'][int(result)-1]['feedUrl']


