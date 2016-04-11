#include <Receiver.cpp>
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
    status = ::bind(s,(struct sockaddr*) &address, sizeof(address));
    
    if (status != 0) {
        cerr << "server: Bind failed, unable to create port." << endl;
        exit(-1);
    }
    
    receiveFile(s, address);

    close(s);
  
    return 0;
}
