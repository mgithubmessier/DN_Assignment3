#include <string>
using namespace std;
const int BUFFER_SIZE = 1024;
class DataPacket
{
public:
  char buffer[BUFFER_SIZE];
  int sequenceNumber;
  unsigned int checksum;
  void computeChecksum();
  void print();
};