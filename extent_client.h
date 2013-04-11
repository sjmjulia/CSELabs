// extent client interface.

#ifndef extent_client_h
#define extent_client_h

#include <string>
#include "extent_protocol.h"
#define BSIZE 512
class extent_client {
 private:
  rpcc *cl;
    //filling begin
        int fd;
        int mutex;
    //filling end
 public:
  extent_client(std::string dst);

  extent_protocol::status get(extent_protocol::extentid_t eid, 
			      char* buf);
  extent_protocol::status put(extent_protocol::extentid_t eid, char* buf);
  extent_protocol::status getattr(extent_protocol::extentid_t eid, extent_protocol::attr &attr);
  extent_protocol::status remove(extent_protocol::extentid_t eid);
        //filling begin
        ~extent_client();
        //filling end
};

#endif 

