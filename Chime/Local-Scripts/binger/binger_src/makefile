OBJTEST= servercom.o overview.o artio.o util.o nzbparse.o decoders.o artreader.o subjparser.o
OBJ= servercom.o overview.o artio.o artutils.o util.o arthandler.o nzbparse.o decoders.o artreader.o subjparser.o post.o queue.o
ARTINCL= artdata.h artdata.p
SERVINCL= server.h server.p

prod:
	make binger FLAGS="-Wall -Wunused"

debug:
	make binger FLAGS="-g -Wall -Wunused"

binger:main.o $(OBJ)
	cc -o binger $(FLAGS) main.o $(OBJ) -lpthread -lcurses -lssl -lcrypto

#ssl_proxy is just used with the ypost program to give an ssl posting
ssl_proxy:ssl_proxy.c
	cc -o ssl_proxy ssl_proxy.c -lssl -lcrypto

test1:test1.o $(OBJTEST)
	cc -o test1 test1.o $(OBJTEST) -lpthread -lcurses -lssl

servercom.o:servercom.c common.h server.h server.p
	cc -c $(FLAGS) servercom.c

overview.o:overview.c common.h server.p hash.h artdata.p
	cc -c $(FLAGS) overview.c

arthandler.o:arthandler.c common.h server.h hash.h artdata.p
	cc -c $(FLAGS) arthandler.c

decoders.o:decoders.c common.h server.h hash.h artdata.p
	cc -c $(FLAGS) decoders.c

artio.o:artio.c common.h server.p hash.h artdata.p
	cc -c $(FLAGS) artio.c

queue.o:queue.c common.h server.p hash.h artdata.p
	cc -c $(FLAGS) queue.c

main.o:main.c common.h server.h
	cc -c $(FLAGS) main.c

test1.o:test1.c common.h
	cc -c $(FLAGS) test1.c

test2.o:test2.c common.h
	cc -c $(FLAGS) test2.c

subjparser.o:subjparser.c common.h artdata.p
	cc -c $(FLAGS) subjparser.c

util.o:util.c common.h
	cc -c $(FLAGS) util.c

artreader.o:artreader.c common.h artdata.p
	cc -c $(FLAGS) artreader.c

post.o:post.c common.h artdata.p
	cc -c $(FLAGS) post.c

artutils.o:artutils.c common.h artdata.p
	cc -c $(FLAGS) artutils.c

nzbparse.o:nzbparse.c common.h nzbparse.h artdata.p
	cc -c $(FLAGS) nzbparse.c

clean:
	-@rm -f *.o test1 binger

install:
	cp binger *.hlp /usr/local/bin

