#ifndef lock_server_cache_h
#define lock_server_cache_h

#include <string>

#include <set>
#include <map>
#include "lock_protocol.h"
#include "rpc.h"
#include "lock_server.h"


class lock_server_cache {
 private:

  typedef lock_protocol::lockid_t lid_t;
  int nacquire;
  pthread_mutex_t lock_map_mutex;
  std::map<lid_t, bool> lock_map;
  std::map<lid_t, bool> lock_revoked_map;
  std::map<lid_t, std::string> lock_owner_map;
  std::map<lid_t, std::set<std::string> > lock_waiters_map;
  std::map<lid_t, pthread_mutex_t> lock_mutex_map;
  std::map<lid_t, pthread_cond_t> lock_cond_map;

 public:
  lock_server_cache();
  lock_protocol::status stat(lock_protocol::lockid_t, int &);
  int acquire(lock_protocol::lockid_t, std::string id, int &);
  int release(lock_protocol::lockid_t, std::string id, int &);
};

#endif
