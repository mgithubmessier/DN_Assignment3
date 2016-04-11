//Joe Lebedew
//CSC390
//asg3

#include<iostream>

//following inlcudes are for file system api
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>

using namespace std;

#define BUFFERSIZE 1024 //size of segment being read

struct DataPacket
{
  char buffer[BUFFERSIZE];
  int sequenceNumber;
  unsigned int checksum;
};

int sequenceNumber = 0; //number of segments received

void receiveFile(int s, sockaddr_in address, int addressSize)
{
  int bytesRead = 1; //arbitrary number to allow the loop to start, will be override by recv function

  while(bytesRead > 0)
    {
      struct DataPacket data;

      bytesRead = recvfrom(s, (void*)&data, sizeof(data), 0, (struct sockaddr*) &address, (socklen_t *) &addressSize);

      if (bytesRead == 0)
	{
	  break;
	}

      unsigned int checksum = 0;
      char* toCreateChecksum = data.buffer;
      while(*toCreateChecksum)
	{
	  checksum += *toCreateChecksum++;
	}

      if (data.sequenceNumber == sequenceNumber && data.checksum == checksum)
	{
	  sequenceNumber ++;
	  sendto(s, &sequenceNumber, 4, 0, (struct sockaddr*) &address, (socklen_t) addressSize); //send ACK as the next sequence number expected
	  cout<<"File Segment Received Successfully"<<endl;
	}
      else
	{
	  if (data.sequenceNumber != sequenceNumber)
	    {
	      cout<<"Expected file segment " << sequenceNumber << " but received " << data.sequenceNumber << " requesting " << sequenceNumber << " again"<<endl;
	    }
	  else
	    {
	      cout<<"File segment " << sequenceNumber << " was corrupted during send, requesting again"<<endl;
	    }
	  sendto(s, &sequenceNumber, 4, 0, (struct sockaddr*) &address, (socklen_t) addressSize); //send NACK as sequence number expected
	}
    }
}

int main(int argc, char* argv[])
{
    int s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    
    if (s <= 0) {
        cerr << "server: Socket creation failed." << endl;
        exit(-1);
    }

    sockaddr_in address;
    int addressSize = sizeof(address);
    bzero(&address,addressSize);
    address.sin_family=AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port  = htons(15000);
    
    int status=0;
    status = bind(s,(struct sockaddr*) &address, sizeof(address));
    
    if (status != 0) {
        cerr << "server: Bind failed, unable to create port." << endl;
        exit(-1);
    }
    
    receiveFile(s, address, addressSize);

    close(s);
  
    return 0;
}
