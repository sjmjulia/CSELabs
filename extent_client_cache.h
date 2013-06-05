// extent client cache interface.

#ifndef extent_client_cache_h
#define extent_client_cache_h

#include <string>
#include "extent_protocol.h"
#include "extent_client.h"
#include <map>

class extent_client_cache : public extent_client {
 private:
  struct Cache {
    char data_cache[BSIZE];
    extent_protocol::attr attr_cache;
    bool is_data_cached;
    bool is_attr_cached;
    bool is_data_dirty;
    Cache ()
    {
      is_data_cached = false;
      is_attr_cached = false;
    }
  };
  std::map<extent_protocol::extentid_t, Cache> cache_map;
  pthread_mutex_t cache_map_mutex;
 public:
  extent_client_cache(std::string dst);

  extent_protocol::status get(extent_protocol::extentid_t eid, 
			      char* buf);
  extent_protocol::status put(extent_protocol::extentid_t eid, char* buf);
  extent_protocol::status getattr(extent_protocol::extentid_t eid, extent_protocol::attr &attr);
  extent_protocol::status remove(extent_protocol::extentid_t eid);
  extent_protocol::status flush(extent_protocol::extentid_t eid);
  virtual ~extent_client_cache();
};

#endif 

