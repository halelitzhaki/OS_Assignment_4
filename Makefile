default: all

all: st_reactor.so react_server

react_server: react_server.o 
	gcc -o react_server react_server.o ./st_reactor.so

st_reactor.so: st_reactor.o reactor.h
	gcc -shared -o st_reactor.so st_reactor.o

st_reactor.o: st_reactor.c reactor.h
	gcc -c -fPIC st_reactor.c

react_server.o: react_server.c reactor.h
	gcc -c react_server.c

clean:
	rm *.o *.so react_server