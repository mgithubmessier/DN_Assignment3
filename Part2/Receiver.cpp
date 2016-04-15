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
const int WINDOW_SIZE = 1000;
int currentSequenceNumber = 0;
DataPacket outOfSequenceBuffer [WINDOW_SIZE];
int outOfSequenceIndex = 0;

bool isCorrectChecksum(DataPacket packet) {
	return (packet.computeChecksum() == packet.checksum);
}
bool isTimeout(DataPacket packet) {
	string easyToCompare = packet.buffer;
	return (easyToCompare == "not arrive packet");
}
bool receiveDataPacket(int socket, sockaddr_in address, DataPacket packet, ofstream* outputFile) {
	int addressSize;
	string ack = "ACK_", nack = "NACK";

	cout << "\n\nReceiver.cpp:receiveDataPacket()" << endl;
	
	recvfrom(socket, (DataPacket*)&packet, sizeof(packet), 0, 
		(struct sockaddr*) &address, (socklen_t *) &addressSize);
	
	recvfrom(socket, (char*)&packet.buffer, sizeof(packet.buffer), 0, 
		(struct sockaddr*) &address, (socklen_t *) &addressSize);

	recvfrom(socket, (int*)&packet.sequenceNumber, sizeof(packet.sequenceNumber), 0, 
		(struct sockaddr*) &address, (socklen_t *) &addressSize);
	
	recvfrom(socket, (int*)&packet.checksum, sizeof(packet.checksum), 0, 
		(struct sockaddr*) &address, (socklen_t *) &addressSize);

	recvfrom(socket, (bool*)&packet.sent, sizeof(packet.sent), 0, 
		(struct sockaddr*) &address, (socklen_t *) &addressSize);

	recvfrom(socket, (bool*)&packet.written, sizeof(packet.written), 0, 
		(struct sockaddr*) &address, (socklen_t *) &addressSize);

	packet.print("Receiver.cpp:receiveDataPacket():packet");

	cout << "Receiver.cpp:receiveDataPacket(): packet received." << endl;

	cout << "Receiver.cpp:receiveDataPacket(): processing checksum..." << endl;
	
	if(strcmp(packet.buffer,"END") == 0) {
		return false;
	} else if(isTimeout(packet)) {
		cout << "Receiver.cpp:receiveDataPacket(): timeout packet received. No ACK to send." << endl;
		ack = "NACK_" + intToString(packet.sequenceNumber) + ": TIMEOUT\0";
		char easyToSend [100];

		cout << "Receiver.cpp:receiveDataPacket(): ack.c_str(): " << ack.c_str() << endl;
		strcpy(easyToSend,ack.c_str());
		cout << "Receiver.cpp:receiveDataPacket(): sending " << easyToSend << endl;

		sendto(socket, (void*)&easyToSend, sizeof(easyToSend),
			0, (struct sockaddr*) &address, (socklen_t) (sizeof(address)));
	} else if(isCorrectChecksum(packet)) {
		cout << "Receiver.cpp:receiveDataPacket(): checksum is correct." << endl;
		ack = ack + intToString(packet.sequenceNumber) + "\0";
		char easyToSend [100];

		cout << "Receiver.cpp:receiveDataPacket(): ack.c_str(): " << ack.c_str() << endl;
		strcpy(easyToSend,ack.c_str());
		cout << "Receiver.cpp:receiveDataPacket(): sending " << easyToSend << endl;

		sendto(socket, (void*)&easyToSend, sizeof(easyToSend),
			0, (struct sockaddr*) &address, (socklen_t) (sizeof(address)));
		if(currentSequenceNumber == packet.sequenceNumber) {
			cout << "Receiver.cpp:receiveDataPacket():writing in sequence packet to file." << endl;
			cout << "Receiver.cpp:receiveDataPacket():currentSequenceNumber: " << currentSequenceNumber << endl;
			string easyToWrite = packet.buffer;
			(*outputFile) << easyToWrite;
			currentSequenceNumber++;
			if(outOfSequenceIndex > 0) {
				for(int i = 0; i < outOfSequenceIndex; i++) {
					if(outOfSequenceBuffer[i].sequenceNumber == currentSequenceNumber) {
						cout << "Receiver.cpp:receiveDataPacket(): writing out of sequence packet to file." << endl;
						outOfSequenceBuffer[i].print("Receiver.cpp:receiveDataPacket()");
						easyToWrite = outOfSequenceBuffer[i].buffer;
						(*outputFile) << easyToWrite;
						outOfSequenceBuffer[i].written = true;
						currentSequenceNumber++;
					}
				}
				int defrag = 0;
				for(int i = 0; i < outOfSequenceIndex; i++) {
					if(!outOfSequenceBuffer[i].written) {
						outOfSequenceBuffer[defrag++] = outOfSequenceBuffer[i];
					}
				}
				outOfSequenceIndex = defrag;
			}
		} else {
			cout << "Receiver.cpp:receiveDataPacket(): packet out of sequence, " << 
			currentSequenceNumber << " expected" << endl;
			outOfSequenceBuffer[outOfSequenceIndex++] = packet;
		}
	} else {
		cout << "Receiver.cpp:receiveDataPacket(): checksum is incorrect." << endl;
		ack = "NACK_" + intToString(packet.sequenceNumber) + ": INV_CHECKSUM\0";
		char easyToSend [100];
		cout << "Receiver.cpp:receiveDataPacket(): sending " << easyToSend << endl;
		strncpy(easyToSend,ack.c_str(),ack.length());

		sendto(socket, (void*)&easyToSend, sizeof(easyToSend),
			0, (struct sockaddr*) &address, (socklen_t) (sizeof(address)));
	}
	cout << "Receiver.cpp:receiveDataPacket(): "<< ack <<" sent.\n\n" << endl;
	return true;
}
void receiveFile(int socket, sockaddr_in address) {
	ofstream outputFile;
	outputFile.open("output.txt");

	cout << "Receiver.cpp:receiveFile()" << endl;
	DataPacket packet;
	while(receiveDataPacket(socket,address,packet,&outputFile));
}