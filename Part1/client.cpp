//Joe Lebedew
//CSC390
//asg3 stop and wait

#include<iostream>

#include<sys/select.h>
#include<sys/time.h>

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

#define BUFFERSIZE 1024 //file sequence number + checksum + segment

struct DataPacket
{
  char buffer[BUFFERSIZE];
  int sequenceNumber;
  unsigned int checksum;
};

int ACKorNACK; //whether or not the server received the file correctly
int sequenceNumber = 0; //sequence number starts at one for the WHOLE program and is incremented after each send

void sendFile(int s, string toSend, sockaddr_in address, int addressSize)
{
  char buffer[BUFFERSIZE];
  int inputFile;
  int sizeOfInputFile;

  inputFile = open(toSend.c_str(), O_RDONLY);

  if (inputFile < 0) //if the file descriptor is less than zero, there was an error reading the file
    {
      cout<<"Error reading file"<<endl;
    }

  bzero(buffer, BUFFERSIZE);
  sizeOfInputFile = read(inputFile, &buffer, BUFFERSIZE);

  if (sizeOfInputFile < 0) //if read reports less than zero bytes, there was an error reading the file
    {
      cout<<"Error reading file"<<endl;
    }

  while (sizeOfInputFile > 0)
    {
      unsigned int checksum = 0;
      char* toCreateChecksum = buffer;
      while (*toCreateChecksum)
	{
	  checksum += *toCreateChecksum++;
	}

      struct DataPacket data;     
      strncpy(data.buffer, buffer, BUFFERSIZE);
      data.sequenceNumber = sequenceNumber;
      data.checksum = checksum;

      int probability = (rand() % 10 + 1);
      if (probability == 7) //simulate not arriving at receiver
	{
	  //sendto(s, (void*)&data, sizeof(data), 0, (struct sockaddr*) &address, (socklen_t) addressSize);
	}
      else if (probability == 8) //simulate corrupted segment
	{
	  data.checksum = -85635;
	  sendto(s, (void*)&data, sizeof(data), 0, (struct sockaddr*) &address, (socklen_t) addressSize);
	}
      else if (probability == 9) //simulate incorrect order
	{
	  data.sequenceNumber = (rand() % 10000 + 1);
	  sendto(s, (void*)&data, sizeof(data), 0, (struct sockaddr*) &address, (socklen_t) addressSize);
	}
      else
	{      
	  sendto(s, (void*)&data, sizeof(data), 0, (struct sockaddr*) &address, (socklen_t) addressSize);
	}

      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 100000;

      if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
	{
	  cout<<"error creating recvfrom timeout"<<endl;
	}

      int bytesReceived = recvfrom(s, &ACKorNACK, 4, 0, (struct sockaddr*) &address, (socklen_t*) &addressSize);
      
      if (bytesReceived >= 0 && ACKorNACK == (sequenceNumber+1))
	{
	  cout<<"File Segment " << sequenceNumber << " Sent Successfully"<<endl;
	  sequenceNumber++;
	  sizeOfInputFile = read(inputFile, &buffer, BUFFERSIZE);
	}
      else if (bytesReceived < 0)
	{
	  cout<<"Timeout reached trying to send segment " << sequenceNumber << ", trying to resend"<<endl;
	}
      else
	{
	  cout<<"File Segment " << sequenceNumber << " Failed to Send, trying again"<<endl;
	}
    }
  sendto(s, 0, 0, 0, (struct sockaddr*) &address, (socklen_t) addressSize); //after sending file to the receiver, send nothing to signal end of file

  close(inputFile); //close the link to the input file
}

void simulate(int s)
{

}

int main(int argc, char* argv[])
{
    int s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    
    if (s <= 0) {
        cerr << "clientr: Socket creation failed." << endl;
        exit(-1);
    }
    
    sockaddr_in address;
    int addressSize = sizeof(address);
    bzero(&address,addressSize);
    address.sin_family=AF_INET;
    address.sin_port  = htons(15000);
    if (!inet_aton("127.0.0.1",&address.sin_addr))
    {
        cout << "inet_aton() failed" << endl;
    }
  
    sendFile(s, "a.log", address, addressSize);

    //shutdown the client connection
    close(s);
    
    return 0;
}
