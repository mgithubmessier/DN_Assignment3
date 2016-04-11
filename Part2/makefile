CC = g++
CPPFLAGS = -g -I. -std=c++11
all: server client

server: server.o
client: client.o DataPacket.o 

server.o: server.cpp
client.o: client.cpp
DataPacket.o: DataPacket.cpp

clean:
	rm server
	rm client
	rm *.o
