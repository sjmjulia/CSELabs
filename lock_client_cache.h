// lock client interface.

#ifndef lock_client_cache_h

#define lock_client_cache_h

#include <map>
#include <string>
#include "lock_protocol.h"
#include "rpc.h"
#include "lock_client.h"
#include "lang/verify.h"


// Classes that inherit lock_release_user can override dorelease so that 
// that they will be called when lock_client releases a lock.
// You will not need to do anything with this class until Lab 5.
class lock_release_user {
 public:
  virtual void dorelease(lock_protocol::lockid_t) = 0;
  virtual ~lock_release_user() {};
};

class lock_client_cache : public lock_client {
 private:
  class lock_release_user *lu;
  int rlock_port;
  std::string hostname;
  std::string id;
  enum oostatus { NONE, FREE, LOCKED, ACQUIRING, RELEASING, EARLY_REVOKED};
  std::map<int, oostatus> status_map;
  std::map<int, int> waiters_num_map;
  std::map<int, bool> revoked_map;
  std::map<int, bool> retried_map;
  pthread_mutex_t status_map_mutex;
  std::map<int, pthread_mutex_t> status_mutex_map;
  std::map<int, pthread_cond_t> status_cond_map;
 public:
  static int last_port;
  lock_client_cache(std::string xdst, class lock_release_user *l = 0);
  virtual ~lock_client_cache() {};
  lock_protocol::status acquire(lock_protocol::lockid_t);
  lock_protocol::status release(lock_protocol::lockid_t);
  rlock_protocol::status revoke_handler(lock_protocol::lockid_t, 
                                        int &);
  rlock_protocol::status retry_handler(lock_protocol::lockid_t, 
                                       int &);
};


#endif
