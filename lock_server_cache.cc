// the caching lock server implementation

#include "lock_server_cache.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "lang/verify.h"
#include "handle.h"
#include "tprintf.h"


lock_server_cache::lock_server_cache()
{
  pthread_mutex_init(&lock_map_mutex, NULL);
  lock_map.clear();
  lock_revoked_map.clear();
  lock_owner_map.clear();
  lock_waiters_map.clear();
  lock_mutex_map.clear();
  lock_cond_map.clear();
}


int lock_server_cache::acquire(lock_protocol::lockid_t lid, std::string id, 
                               int &)
{
  tprintf("acquire -> lid %d id %s\n", lid, id.c_str());
  lock_protocol::status ret = lock_protocol::OK;
  std::string dst;
  handle h("");
  rpcc *cl = NULL;
  int r;
  //create if not exists :-)
  pthread_mutex_lock(&lock_map_mutex);
  std::map<lid_t, bool>::iterator it;

  it = lock_map.find(lid);
  if (lock_map.end() == it) {
    lock_map[lid] = false;
    lock_revoked_map[lid] = false;
    lock_owner_map[lid] = "";
    lock_waiters_map[lid];
    lock_mutex_map[lid];
    lock_cond_map[lid];
    pthread_mutex_init(&lock_mutex_map[lid], NULL);
    pthread_cond_init(&lock_cond_map[lid], NULL);
  }
  pthread_mutex_unlock(&lock_map_mutex);
  //deal with acquire
  pthread_mutex_lock(&lock_mutex_map[lid]);
  tprintf("acquire => lid %d id %s holds the mutex\n", lid, id.c_str());
  if (false == lock_map[lid]) {
    lock_map[lid] = true;
    lock_owner_map[lid] = id;
    //delete no matter in waiters_map[lid] or not
    lock_waiters_map[lid].erase(id);
    lock_revoked_map[lid] = false;
    tprintf("acquire => lid %d id %s acquires the lock\n", lid, id.c_str());
    goto RET_POINT;
  }
  if (lock_owner_map[lid] == id) {
    //he already owns the lock...
    tprintf("acquire => lid %d id %s I already owns the lock\n", lid, id.c_str());
    goto RET_POINT;
  }

  //somebody owns the lock
  //tha acquiring client doesn't own the lock
  lock_waiters_map[lid].insert(id);
  ret = lock_protocol::RETRY;
  if (lock_revoked_map[lid]) {
    tprintf("acquire => lid %d id %s others are revoking the lock\n", lid, id.c_str());
    goto RET_POINT;
  }
  lock_revoked_map[lid] = true;
  pthread_mutex_unlock(&lock_mutex_map[lid]);
  tprintf("acquire => lid %d id %s returns the mutex\n", lid, id.c_str());
  //revoke from the owner
  //////////////
  dst = lock_owner_map[lid];
  h = handle(dst);
  cl = h.safebind();
  if (!cl)
    VERIFY(0);
  tprintf("acquire => lid %d id %s call %s revoke rpc\n", lid, id.c_str(), dst.c_str());
  cl->call(rlock_protocol::revoke, lid, r);
  cl = NULL;
  return ret;

RET_POINT:
  pthread_mutex_unlock(&lock_mutex_map[lid]);
  tprintf("acquire => lid %d id %s returns the mutex\n", lid, id.c_str());
  return ret;
}

int 
lock_server_cache::release(lock_protocol::lockid_t lid, std::string id, 
         int &r)
{
  tprintf("release --> lid %d id %s\n", lid, id.c_str());
  lock_protocol::status ret = lock_protocol::OK;
  std::string dst, dst2;
  handle h("");
  rpcc *cl;
  int rr;
  //create if not exists :-)
  pthread_mutex_lock(&lock_map_mutex);
  std::map<lid_t, bool>::iterator it;
  it = lock_map.find(lid);
  if (lock_map.end() == it)
    VERIFY(0);
  pthread_mutex_unlock(&lock_map_mutex);
  pthread_mutex_lock(&lock_mutex_map[lid]);
  tprintf("release ==> lid %d id %s holds the thread\n", lid, id.c_str());
  lock_map[lid] = false;
  lock_owner_map[lid] = "";
  lock_revoked_map[lid] = false;
  /*
  if (lock_waiters_map[lid].empty()) {
    pthread_mutex_unlock(&lock_mutex_map[lid]);
    return ret;
  }
  */
  std::set<std::string> lock_waiters = lock_waiters_map[lid];
  //unlock
  pthread_mutex_unlock(&lock_mutex_map[lid]);
  tprintf("release ==> lid %d id %s returns the thread\n", lid, id.c_str());
  //send retry
  std::set<std::string>::iterator waiter;
  int cnt = 2;
  for (waiter=lock_waiters.begin();
          waiter!=lock_waiters.end();
          ++waiter) {
      if (cnt-- == 0) break;
      h = handle(*waiter);
      cl = h.safebind();
      if (!cl)
          VERIFY(0);
      tprintf("release ==> lid %d id %s call %s retry rpc\n", lid, id.c_str(), waiter->c_str());
      cl->call(rlock_protocol::retry, lid, rr);
  }
//RET_POINT:
  return ret;
}

lock_protocol::status
lock_server_cache::stat(lock_protocol::lockid_t lid, int &r)
{
  tprintf("stat request\n");
  r = nacquire;
  return lock_protocol::OK;
}

