#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <fstream>

#include <DataPacket.h>

using namespace std;
const int 	PACKET_SIZE = 1024,
			WINDOW_SIZE = 4;
int fileSize = 0,
	numCharsRead = 0;
bool readNextDataPacket(int inputFile, char* packet, int position) {
	cout << "Sender.cpp:readNextDataPacket()" << endl;
	if(inputFile >= 0) {
		bzero(packet,PACKET_SIZE);
	    numCharsRead = read(inputFile,packet,PACKET_SIZE-1);
	    packet[PACKET_SIZE-1] = '\0';
	    if(numCharsRead < 0)
	    	return false;
		//cout << "Sender.cpp:readNextDataPacket():packet: " << packet << endl;
		return true;
	}
	return false;
}

DataPacket* createWindow(DataPacket* window, int shift, int inputFile) {
	int currentPosition = 0;
	char nextPacket [PACKET_SIZE];
	for(int i = 0; i < WINDOW_SIZE; i++) {
		if(readNextDataPacket(inputFile,nextPacket,currentPosition)) {
			//set the contents of the next free data packet in the window
			strncpy(window[i].buffer,nextPacket,PACKET_SIZE);
			window[i].sequenceNumber = (shift + (i*4));
			window[i].computeChecksum();
			cout << "Sender.cpp:createWindow():nextPacket: \n" << nextPacket << endl;
		} else
			break;
	}
	return window;
}
void sendFile(int socket, string filename, sockaddr_in address) {
	int inputFile = open("a.txt",O_RDONLY);
	char buffer [PACKET_SIZE];

	DataPacket window [WINDOW_SIZE];
	createWindow(window,0,inputFile);
	//send window
	//shift window as acks are received
	close(inputFile);


}