#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>

#include <Reader.cpp>

int windowStart = 0;
void sendDataPacket(int socket, sockaddr_in address, DataPacket packet, bool override) {	
	cout << "\n\nSender.cpp:sendDataPacket()" << endl;
	int probability;

	probability = (rand() % 10 + 1);
	if(override)
		probability = 4;
	if(probability == 1) {
		packet = packet.getCorruptPacket();
		cout << "Sender.cpp:sendDataPacket():packet type: corrupt" << endl;
	} else if(probability == 2) {
		packet = packet.getNotArrivePacket();
		cout << "Sender.cpp:sendDataPacket():packet type: not arrive" << endl;
	} else if(probability == 3) {
		cout << "Sender.cpp:sendDataPacket():packet type: out of order" << endl;
	} else {
		cout << "Sender.cpp:sendDataPacket():packet type: normal" << endl;
	}
	cout << "Sender.cpp:sendDataPacket(): sending...";
	packet.print("");
	sendto(socket, (void*)&packet, sizeof(packet), 
		0, (struct sockaddr*) &address, (socklen_t) (sizeof(address)));
	sendto(socket, (void*)&packet.buffer, sizeof(packet.buffer),
		0, (struct sockaddr*) &address, (socklen_t) (sizeof(address)));
	sendto(socket, (void*)&packet.sequenceNumber, sizeof(packet.sequenceNumber),
		0, (struct sockaddr*) &address, (socklen_t) (sizeof(address)));
	sendto(socket, (void*)&packet.checksum, sizeof(packet.checksum),
		0, (struct sockaddr*) &address, (socklen_t) (sizeof(address)));
	sendto(socket, (void*)&packet.sent, sizeof(packet.sent),
		0, (struct sockaddr*) &address, (socklen_t) (sizeof(address)));
	sendto(socket, (void*)&packet.written, sizeof(packet.written),
		0, (struct sockaddr*) &address, (socklen_t) (sizeof(address)));
	cout << "Sender.cpp:sendDataPacket(): packet sent." << endl;
}
string ackDataPacket(int socket, sockaddr_in address, DataPacket packet) {
	cout << "Sender.cpp:ackDataPacket(): waiting for ACK_" << packet.sequenceNumber << endl;
	char ack [100];
	int bytesReceived = recvfrom(socket, (char*)&ack, sizeof(ack), 0, 
		(struct sockaddr*) &address, (socklen_t *) &address);
	cout << "Sender.cpp:ackDataPacket():received " << ack << endl;
	return ack;
}
void clearTimeouts(int socket, sockaddr_in address, DataPacket* window, 
	int* winTimeoutIndices,int numTimeouts) {

	cout << "Sender.cpp:clearTimeouts()" << endl;
	DataPacket nextTimeoutPacket;
	string ack;
	int timeoutsLeft = numTimeouts, index = 0;
	while(timeoutsLeft > 0) {
		cout << "Sender.cpp:clearTimeouts():timeoutsLeft: " << timeoutsLeft << endl;
		nextTimeoutPacket = window[winTimeoutIndices[index]];
		do {
			sendDataPacket(socket,address,nextTimeoutPacket,false);
			ack = ackDataPacket(socket,address,nextTimeoutPacket);
		} while((ack.substr(ack.length()-13) == "INV_CHECKSUM") || 
			(ack.substr(ack.length()-8) == "TIMEOUT"));

		window[winTimeoutIndices[index]].print("Sender.cpp:clearTimeouts(): successfully sent");
		window[winTimeoutIndices[index++]].sent = true;
		cout << "Sender.cpp:clearTimeouts():ack: " << ack << endl;
		timeoutsLeft--;
	}
}

void swap(DataPacket* window, int swap1, int swap2) {
	cout << "Sender.cpp:swap()" << endl;
	if(swap2 < WINDOW_SIZE) {
		DataPacket temp = window[swap1];
		window[swap1] = window[swap2];
		window[swap2] = temp;
	}
}
void insertNewDataPacket(DataPacket newDataPacket,DataPacket* window) {
	cout << "Sender.cpp:insertNewDataPacket()" << endl;
	for(int i = 0; i < WINDOW_SIZE; i++) {
		if(window[i].sent) {
			if(i == 0) {
				window[i] = newDataPacket;
				return;
			} else {
				for(int j = i; j > 1; j++)
					window[j] = window[j-1];
				window[0] = newDataPacket;
				return;
			}
		}
	}
}
void sendWindow(int socket, sockaddr_in address, int* inputFile, 
	DataPacket* window, int* sequenceNumber, int* numCharsRead) {

	cout << "Sender.cpp:sendWindow()" << endl;
	string ack;
	int winTimeoutIndices [WINDOW_SIZE];
	int noACKIndex = 0;
	int probability;
	for(int i = 0; i < WINDOW_SIZE; i++) {
		probability = (rand() % 10 + 1);
		if(probability == 1) 
			swap(window,i,rand()%(WINDOW_SIZE-i)+i);
		sendDataPacket(socket, address, window[i],false);
		ack = ackDataPacket(socket, address, window[i]);
		
		if((ack.length() > 13) && (ack.substr(ack.length()-13) == "INV_CHECKSUM")) {
			cout << "Sender.cpp:sendWindow(): checksum was invalid. Resending corrupt packet." << endl;
			i--;
		} else if ((ack.length() > 8) && (ack.substr(ack.length()-8) == "TIMEOUT")) {
			cout << "Sender.cpp:sendWindow(): timeout ack" << endl;
			winTimeoutIndices[noACKIndex++] = i;
		} else {
			cout << "Sender.cpp:sendWindow(): successful send." << endl;
			window[i].sent = true;

			cout << "Sender.cpp:sendWindow(): reading new packet." << endl;
			DataPacket newDataPacket;
			(*numCharsRead) += readNextDataPacket(&newDataPacket,inputFile,sequenceNumber);
			cout << "Sender.cpp:sendFile():sequenceNumber changed in readNextDataPacket() to: " << *sequenceNumber << endl;
			cout << "Sender.cpp:sendWindow():numCharsRead: " << *numCharsRead << endl;
			
			cout << "Sender.cpp:sendWindow(): loading new packet into window." << endl;
			insertNewDataPacket(newDataPacket,window);
			//push all packets in the window back one
			//fill the end of the window with the next data packet
		}
		cout << "\n\n" << endl;
		if(i%9 == 0) {
			cout << "Sender.cpp:sendWindow(): clearing leftover packets with no ack due to timeout" << endl;
			clearTimeouts(socket,address,window,winTimeoutIndices,noACKIndex);
			noACKIndex = 0;
		}
	}
}
void sendFile(int socket, string filename, sockaddr_in address) {
	ifstream file( "input.txt", ios::binary | ios::ate);
	int fileSize = file.tellg();
	//cout << "Sender.cpp:sendFile():fileSize: " << fileSize << endl;
	file.close();

	int inputFile = open("input.txt",O_RDONLY);
	char buffer [PACKET_SIZE];
	DataPacket window [WINDOW_SIZE];
	int sequenceNumber = 0;
	cout << "Sender.cpp:sendFile():sequenceNumber instantiated at: " << sequenceNumber << endl;
	int numCharsRead = createWindow(window,&inputFile,&sequenceNumber);
	cout << "Sender.cpp:sendFile():sequenceNumber changed in createWindow() to: " << sequenceNumber << endl;

	while(numCharsRead < fileSize) {
		sendWindow(socket, address, &inputFile, window, &sequenceNumber, &numCharsRead);
	}
	DataPacket endPacket;
	endPacket = endPacket.getEndPacket();
	sendDataPacket(socket,address,endPacket,true);
	close(inputFile);
}