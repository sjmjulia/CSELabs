// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_server::lock_server():
  nacquire (0)
{
    lock_status.clear();
    lock_times.clear();
    pthread_mutex_init(&mutex, NULL);
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
    pthread_mutex_lock(&mutex);
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  //r = nacquire;
  r = lock_times[lid];
    pthread_mutex_unlock(&mutex);
  return ret;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;

    while (1) {
        pthread_mutex_lock(&mutex);
        if (!lock_status[lid]) {
            lock_status[lid] = true;
            lock_times[lid]++;
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }
  
  return ret;
}


lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;

    pthread_mutex_lock(&mutex);
    if (!lock_status[lid]) {
        ret = lock_protocol::NOENT;
    }
    lock_status[lid] = false;
    pthread_mutex_unlock(&mutex);
  
  return ret;
}


