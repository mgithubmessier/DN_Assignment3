#include <DataPacket.h>
#include <iostream>
#include <sstream>

DataPacket DataPacket::getCorruptPacket() {
	DataPacket corruptPacket;
	strncpy(corruptPacket.buffer,"corrupt packet\0",15);
	corruptPacket.sequenceNumber = -1;
	corruptPacket.checksum = -1;
	return corruptPacket;
}
DataPacket DataPacket::getNotArrivePacket() {
	DataPacket notArrivePacket;
	strncpy(notArrivePacket.buffer,"not arrive packet\0",18);
	notArrivePacket.sequenceNumber = -1;
	notArrivePacket.checksum = -1;
	return notArrivePacket;
}
DataPacket DataPacket::getEndPacket() {
	DataPacket endPacket;
	strncpy(endPacket.buffer,"END\0",4);
	endPacket.checksum = endPacket.computeChecksum();
	return endPacket;
}
int DataPacket::computeChecksum() {
	int newChecksum = 0;
	for(int i = 0; i < BUFFER_SIZE; i++) 
		newChecksum+= buffer[i];
	return newChecksum;
}
void DataPacket::print(string message) {
	cout << message << ":" << endl;
	cout << "checksum: " << checksum << endl;
	cout << "sequenceNumber: " << sequenceNumber << endl;
	cout << "buffer: \n" << buffer << endl;
	cout << "sent: " << sent << endl;
	cout << "written: " << written << endl; 
}
string intToString(int i) {
  stringstream ss;
  ss << i;
  return ss.str();
}