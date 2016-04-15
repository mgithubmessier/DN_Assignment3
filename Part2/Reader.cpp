#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <DataPacket.h>

using namespace std;
const int 	PACKET_SIZE = 1024,
			WINDOW_SIZE = 1000;

int readNextData(int* inputFile, char* data) {
	cout << "Reader.cpp:readNextData()" << endl;
	int numCharsRead = 0;
	//cout << "Reader.cpp:readNextData():inputFile: " << inputFile << endl;
	if(inputFile >= 0) {
		bzero(data,PACKET_SIZE);
	    numCharsRead = read(*inputFile,data,PACKET_SIZE-1);
	    data[PACKET_SIZE-1] = '\0';
		//cout << "Reader.cpp:readNextData():data: " << data << endl;
	    if(numCharsRead < 0)
	    	return -1;
		//cout << "Reader.cpp:readNextDataPacket():packet: " << packet << endl;
		return numCharsRead;
	}
	return -1;
}

int readNextDataPacket(DataPacket* packet, int* inputFile, int* sequenceNumber) {
	cout << "Reader.cpp:readNextDataPacket()" << endl;
	int numCharsRead = readNextData(inputFile,(*packet).buffer);
	if(numCharsRead > 0) {
		(*packet).sequenceNumber = (*sequenceNumber)++;
		(*packet).checksum = (*packet).computeChecksum();
		cout << "Reader.cpp:readNextDataPacket():sequenceNumber: " << *sequenceNumber << endl;
		return numCharsRead; 
	} else {
		return -1;
	}
} 
int createWindow(DataPacket* window, int* inputFile, int* sequenceNumber) {
	cout << "Reader.cpp:createWindow()" << endl;
	int numCharsRead = 0;
	for(int i = 0; i < WINDOW_SIZE; i++) {
		numCharsRead += readNextDataPacket(&window[i], inputFile, sequenceNumber);
	}
	return numCharsRead;
}