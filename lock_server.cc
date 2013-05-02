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
    pthread_cond_init(&cv, NULL);
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
    printf("ac\n");
    lock_protocol::status ret = lock_protocol::OK;

    pthread_mutex_lock(&mutex);
    if (lock_status.end() != lock_status.find(lid)) {
        while (lock_status[lid]) {
            printf("cond_wait\n");
            pthread_cond_wait(&cv, &mutex);
        }
    }
    lock_status[lid] = true;
    lock_times[lid]++;

    pthread_mutex_unlock(&mutex);
    return ret;
}


lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
    printf("re\n");
    lock_protocol::status ret = lock_protocol::OK;

    pthread_mutex_lock(&mutex);
    if (lock_status.end() == lock_status.find(lid)) {
        ret = lock_protocol::NOENT;
        goto over;
    }
    if (!lock_status[lid]) {
        ret = lock_protocol::NOENT;
        goto over;
    }
    lock_status[lid] = false;
    pthread_cond_broadcast(&cv);
over:
    pthread_mutex_unlock(&mutex);
  
    return ret;
}


