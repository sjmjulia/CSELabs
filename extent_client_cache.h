// extent client cache interface.

#ifndef extent_client_cache_h
#define extent_client_cache_h

#include <string>
#include "extent_protocol.h"
#include "extent_client.h"

class extent_client_cache : extent_client {
 private:
  rpcc *cl;
  int fd;
  int mutex;
 public:
  extent_client_cache(std::string dst);

  extent_protocol::status get(extent_protocol::extentid_t eid, 
			      char* buf);
  extent_protocol::status put(extent_protocol::extentid_t eid, char* buf);
  extent_protocol::status getattr(extent_protocol::extentid_t eid, extent_protocol::attr &attr);
  extent_protocol::status remove(extent_protocol::extentid_t eid);
  virtual ~extent_client_cache();
};

#endif 

