#include <string>
using namespace std;
const int BUFFER_SIZE = 1024;
class DataPacket
{
public:
  char buffer[BUFFER_SIZE];
  int sequenceNumber;
  unsigned int checksum;
  int computeChecksum();
  void print(string message);
  const char* stringToCharBuffer(string zString);
  DataPacket getCorruptPacket();
  DataPacket getNotArrivePacket();
  DataPacket getEndPacket();
  bool sent = false;
  bool written = false;
};
string intToString(int i);
