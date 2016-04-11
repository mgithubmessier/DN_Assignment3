#include <DataPacket.h>
#include <iostream>
void DataPacket::computeChecksum() {
	for(int i = 0; i < BUFFER_SIZE; i++) 
		checksum+= buffer[i];
}
void DataPacket::print() {
	cout << "checksum: " << checksum << endl;
	cout << "sequenceNumber: " << sequenceNumber << endl;
	cout << "buffer: " << buffer << endl;
}