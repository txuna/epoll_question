LDLIBS = -lpthread

all: server

server: main.o sock.o epoll_event.o
	g++ -o server -g main.o sock.o epoll_event.o $(LDLIBS)

main.o: main.hpp main.cpp
	g++ -c -o main.o main.cpp

sock.o: sock.hpp sock.cpp
	g++ -c -o sock.o sock.cpp

epoll_event.o: epoll_event.hpp epoll_event.cpp
	g++ -c -o epoll_event.o epoll_event.cpp

clean:
	rm *.o server