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

  pthread_mutex_init(&status_map_mutex, NULL);
  status_map.clear();
  status_cv_map.clear();
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
  int ret = lock_protocol::OK;
  int r;
  pthread_mutex_lock(&status_map_mutex);
  std::map<int, oostatus>::iterator it;
  it = status_map.find(lid);
  if (status_map::end == it) {
    //if not exists, create it!
    status_map[lid] = NONE;
    status_cv_map[lid];
    pthread_cond_init(&status_cv_map[lid]);
  }
CHECK_AGAIN:
  switch (status_map[lid]) {
  case NONE:
    status_map[lid] = ACQUIRING;
    pthread_mutex_unlock(&status_map_mutex);
    ret = cl->call(lock_protocol::acquire, cl->id(), lid, r);
    pthread_mutex_lock(&status_map_mutex);
    if (OK == ret) {
      if (NONE == status_map[lid]) 
        status_map[lid] = FREE;
        //do not need to send signals
        //  because this thread will check again.
    }
    goto CHECK_AGAIN;
  case FREE:
    //I got the lock :-)
    status_map[lid] = LOCKED;
    goto RET_POINT;
  case LOCKED:
    //wait someone release the lock and send signal to me
    pthread_cond_wait(&status_cv_map[lid], &status_map_mutex);
    goto CHECK_AGAIN;
  case ACQUIRING:
    //wait to receive reply from server
    //  and the handler sends signal to me
    pthread_cond_wait(&status_cv_map[lid], &status_map_mutex);
    goto CHECK_AGAIN;
  case RELEASING:
    //wait to be granted the lock next time
    pthread_cond_wait(&status_cv_map[lid], &status_map_mutex);
    goto CHECK_AGAIN;
  default:
    VERIFY(0);
  }
RET_POINT:
  pthread_mutex_unlock(&status_map_mutex);
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
  int ret = OK;
  int r;
  pthread_mutex_lock(&status_map_mutex);
  std::map<int, oostatus>::iterator it;
  it = status_map.find(lid);
  if (status_map::end == it)
      VERIFY(0);
  switch (status_map[lid]) {
  case NONE:
    VERIFY(0);
  case FREE:
    VERIFY(0);
  case LOCKED:
    //release the lock and send signal
    status_map[lid] = FREE;
    pthread_cond_signal(&status_cv_map[lid]);
    goto RET_POINT;
  case ACQUIRING:
    VERIFY(0);
  case RELEASING:
    //release rpc to server
    status_map[lid] = NONE;
    pthread_mutex_unlock(&status_map_mutex);
    ret = cl->call(lock_protocol::release, cl->id(), lid, r);
    pthread_mutex_lock(&status_map_mutex);
    goto RET_POINT;
  default:
    VERIFY(0);
  }
RET_POINT:
  pthread_mutex_unlock(&status_map_mutex);
  return lock_protocol::OK;
}

rlock_protocol::status
lock_client_cache::revoke_handler(lock_protocol::lockid_t lid, 
                                  int &)
{
  int ret = rlock_protocol::OK;
  int r;
  pthread_mutex_lock(&status_map_mutex);
  switch (status_map[lid]) {
  case NONE:
    VERIFY(0);
  case FREE:
    pthread_mutex_unlock(&status_map_mutex);
    ret = cl->call(lock_protocol::release, cl->id(), lid, r);
    pthread_mutex_lock(&status_map_mutex);
    goto RET_POINT;
  case LOCKED:
    //set the status to RELEASING
    status_map[lid] = RELEASING;
    goto RET_POINT;
  case ACQUIRING:
    VERIFY(0);
  case RELEASING:
    //seems wrong code
    VERIFY(0);
  default:
    VERIFY(0);
  }
RET_POINT:
  pthread_mutex_unlock(&status_map_mutex);
  return ret;
}

rlock_protocol::status
lock_client_cache::retry_handler(lock_protocol::lockid_t lid, 
                                 int &)
{
  int ret = rlock_protocol::OK;
  pthread_cond_signal(&status_cv_map[lid]);
  return ret;
}



