#include <Sender.cpp>

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
  
    sendFile(s, "a.log", address);

    //shutdown the client connection
    close(s);
    
    return 0;
}