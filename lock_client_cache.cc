// RPC stubs for clients to talk to lock_server, and cache the locks
// see lock_client.cache.h for protocol details.

#include "lock_client_cache.h"
#include "rpc.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include "tprintf.h"


int lock_client_cache::last_port = 0;

lock_client_cache::lock_client_cache(std::string xdst, 
				     class lock_release_user *_lu)
  : lock_client(xdst), lu(_lu)
{
  srand(time(NULL)^last_port);
  rlock_port = ((rand()%32000) | (0x1 << 10));
  const char *hname;
  // VERIFY(gethostname(hname, 100) == 0);
  hname = "127.0.0.1";
  std::ostringstream host;
  host << hname << ":" << rlock_port;
  id = host.str();
  last_port = rlock_port;
  rpcs *rlsrpc = new rpcs(rlock_port);
  rlsrpc->reg(rlock_protocol::revoke, this, &lock_client_cache::revoke_handler);
  rlsrpc->reg(rlock_protocol::retry, this, &lock_client_cache::retry_handler);

  status_map.clear();
  revoked_map.clear();
  retried_map.clear();
  waiters_num_map.clear();
  status_mutex_map.clear();
  status_cond_map.clear();
}

/*  Client
 *  when a thread acquires a lock
 *      none: acquire rpc to s
 *      free: grant the lock
 *      locked: waiting until the lock is freed
 *      acquring: waiting
 *      releasing: waiting
 */
lock_protocol::status
lock_client_cache::acquire(lock_protocol::lockid_t lid)
{
    tprintf("%s >> acquire -----> lid %d\n", id.c_str(), lid);
  int ret = lock_protocol::OK;
  int r;
  //if not exists, create it!
  pthread_mutex_lock(&status_map_mutex);
  std::map<int, oostatus>::iterator it;
  it = status_map.find(lid);
  if (status_map.end() == it) {
    status_map[lid] = NONE;
    status_mutex_map[lid];
    pthread_mutex_init(&status_mutex_map[lid], NULL);
    status_cond_map[lid];
    pthread_cond_init(&status_cond_map[lid], NULL);
  }
  pthread_mutex_unlock(&status_map_mutex);
  //deal with acquire
  pthread_mutex_lock(&status_mutex_map[lid]);
  tprintf("%s >> acquire =====> status %d\n", id.c_str(), status_map[lid]);
CHECK_AGAIN:
  switch (status_map[lid]) {
  case NONE:
    status_map[lid] = ACQUIRING;
    tprintf("%s >> acquire =====> set status from %d to %d\n", id.c_str(), NONE, status_map[lid]);
    pthread_mutex_unlock(&status_mutex_map[lid]);
    tprintf("%s >> acquire =====> unlock to call acquire rpc\n", id.c_str());
    ret = cl->call(lock_protocol::acquire, lid, id, r);
    pthread_mutex_lock(&status_mutex_map[lid]);
    tprintf("%s >> acquire =====> lock to deal with acquire rpc response\n", id.c_str());
    if (lock_protocol::OK == ret) {
      if (ACQUIRING == status_map[lid]) {// || NONE == status_map[lid]) {
        status_map[lid] = FREE;
        tprintf("%s >> acquire =====> set status from %d to %d\n", id.c_str(), ACQUIRING, status_map[lid]);
      }      
    }
    goto CHECK_AGAIN;
  case FREE:
    //I got the lock :)
    status_map[lid] = LOCKED;
    tprintf("%s >> acquire =====> set status from %d to %d\n", id.c_str(), FREE, status_map[lid]);
    goto RET_POINT;
  case LOCKED:
    //wait someone release the lock and send signal to me
  case ACQUIRING:
    //wait to receive reply from server and the handler sends signal to me
    if (ACQUIRING == status_map[lid] && retried_map[lid]) {
        status_map[lid] = NONE;
        retried_map[lid] = false;
        tprintf("%s >> acquire =====> set status from %d to %d\n", id.c_str(), ACQUIRING, status_map[lid]);
        goto CHECK_AGAIN;
    }
  case RELEASING:
    //wait to be granted the lock next time
    tprintf("%s >> acquire =====> cond wait\n", id.c_str());
    ++waiters_num_map[lid];
    pthread_cond_wait(&status_cond_map[lid], &status_mutex_map[lid]);
    --waiters_num_map[lid];
    tprintf("%s >> acquire =====> wake from cond wait\n", id.c_str());
    goto CHECK_AGAIN;
  default:
    VERIFY(0);
  }
RET_POINT:
  pthread_mutex_unlock(&status_mutex_map[lid]);
  tprintf("%s >> acquire =====> unlock\n", id.c_str());
  return lock_protocol::OK;
}

/*  Client
 *  when a thread releases a lock
 *      none: error!
 *      free: error!
 *      locked: release it and notify other threads who acquires
 *      acquring: error! 
 *      releasing: release rpc to s
 */
lock_protocol::status
lock_client_cache::release(lock_protocol::lockid_t lid)
{
  tprintf("%s >> release ---> lid %d\n", id.c_str(), lid);
  int ret = lock_protocol::OK;
  int r;
  //check existance
  pthread_mutex_lock(&status_map_mutex);
  std::map<int, oostatus>::iterator it;
  it = status_map.find(lid);
  if (status_map.end() == it)
      VERIFY(0);
  pthread_mutex_unlock(&status_map_mutex);
  //deal with release
  pthread_mutex_lock(&status_mutex_map[lid]);
  tprintf("%s >> release ===> lock\n", id.c_str());
CHECK_POINT:
  tprintf("%s >> release ===> status %d\n", id.c_str(), status_map[lid]);
  switch (status_map[lid]) {
  case NONE:
    VERIFY(0);
  case FREE:
    VERIFY(0);
  case LOCKED:
    //release the lock and send signal
    if (revoked_map[lid] && 0 == waiters_num_map[lid]) {
      status_map[lid] = RELEASING;
      revoked_map[lid] = false;
      tprintf("%s >> release ===> set status from %d to %d\n", id.c_str(), LOCKED, status_map[lid]);
      //============flush cache here
      if (lu) lu->dorelease(lid);
      tprintf("%s >> release ===> unlock to call release\n", id.c_str());
      pthread_mutex_unlock(&status_mutex_map[lid]);
      ret = cl->call(lock_protocol::release, lid, id, r);
      pthread_mutex_lock(&status_mutex_map[lid]);
      tprintf("%s >> release ===> lock to deal with release rpc response\n", id.c_str());
      status_map[lid] = NONE;
      tprintf("%s >> release ===> set status from %d to %d\n", id.c_str(), RELEASING, status_map[lid]);
    } else {
      status_map[lid] = FREE;
      tprintf("%s >> release ===> set status from %d to %d\n", id.c_str(), LOCKED, status_map[lid]);
    }
    tprintf("%s >> release ===> cond_signal %d\n", id.c_str(), lid);
    pthread_cond_signal(&status_cond_map[lid]);
    goto RET_POINT;
  case ACQUIRING:
    VERIFY(0);
  case RELEASING:
    tprintf("%s >> release =====> cond wait\n", id.c_str());
    ++waiters_num_map[lid];
    pthread_cond_wait(&status_cond_map[lid], &status_mutex_map[lid]);
    --waiters_num_map[lid];
    tprintf("%s >> release =====> wake from cond wait\n", id.c_str());
    goto CHECK_POINT;
  default:
    VERIFY(0);
  }
RET_POINT:
  pthread_mutex_unlock(&status_mutex_map[lid]);
  tprintf("%s >> release ===> unlock\n", id.c_str());
  return ret;
}

rlock_protocol::status
lock_client_cache::revoke_handler(lock_protocol::lockid_t lid, 
                                  int &)
{
  tprintf("%s >> revoke ---> lid %d\n", id.c_str(), lid);
  int ret = rlock_protocol::OK;
  int r;
  pthread_mutex_lock(&status_mutex_map[lid]);
  tprintf("%s >> revoke ===> status %d\n", id.c_str(), status_map[lid]);
  if (FREE == status_map[lid]) {
    tprintf("%s >> revoke ===> set status from %d to %d\n", id.c_str(), status_map[lid], NONE);
    status_map[lid] = NONE;
    tprintf("%s >> revoke ===> call release\n", id.c_str());
    //============flush cache here
    if (lu) lu->dorelease(lid);
    ret = cl->call(lock_protocol::release, lid, id, r);
  } else {
    tprintf("%s >> revoke ===> set revoked %d true\n", id.c_str(), lid);
    revoked_map[lid] = true;
  }
  tprintf("%s >> revoke ===> cond_broadcast %d\n", id.c_str(), lid);
  pthread_cond_broadcast(&status_cond_map[lid]);
RET_POINT:
  pthread_mutex_unlock(&status_mutex_map[lid]);
  tprintf("%s >> revoke ===> unlockd\n", id.c_str());
  return ret;
}

rlock_protocol::status
lock_client_cache::retry_handler(lock_protocol::lockid_t lid, 
                                 int &)
{
  tprintf("%s >> retry -> lid %d\n", id.c_str(), lid);
  int ret = rlock_protocol::OK;
  //only when a acquiring thread enter the pthread_cond_wait,
  //    can this thread get the lock
  pthread_mutex_lock(&status_mutex_map[lid]);
  tprintf("%s >> retry => status %d\n", id.c_str(), status_map[lid]);
  retried_map[lid] = true;
  tprintf("%s >> retry => set retried true\n", id.c_str());
  tprintf("%s >> retry => cond_broadcast %d\n", id.c_str(), lid);
  pthread_cond_broadcast(&status_cond_map[lid]);
  pthread_mutex_unlock(&status_mutex_map[lid]);
  tprintf("%s >> retry => unlock\n", id.c_str());
  return ret;
}



