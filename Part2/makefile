CC = g++
CPPFLAGS = -g -I. -std=c++11
all: server client

server: server.o DataPacket.o
client: client.o DataPacket.o 

server.o: server.cpp
client.o: client.cpp
DataPacket.o: DataPacket.cpp

clean:
	rm *.o
	rm server
	rm client
