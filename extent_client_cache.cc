// RPC stubs for clients to talk to extent_server

#include "extent_client_cache.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include "ttprintf.h"

extent_client_cache::extent_client_cache(std::string dst)
    :extent_client(dst)
{
    /*
  sockaddr_in dstsock;
  make_sockaddr(dst.c_str(), &dstsock);
  cl = new rpcc(dstsock);
  if (cl->bind() != 0) {
    ttprintf("extent_client: bind failed\n");
  }
  */
  cache_map.clear();
  pthread_mutex_init(&cache_map_mutex, NULL);
}
// a demo to show how to use RPC
extent_protocol::status
extent_client_cache::getattr(extent_protocol::extentid_t eid, 
                       extent_protocol::attr &attr)
{
  extent_protocol::status ret = extent_protocol::OK;
  pthread_mutex_lock(&cache_map_mutex);
  Cache &cache = cache_map[eid];
  //check cache
  if (cache.is_attr_cached) {
    attr = cache.attr_cache;
    ttprintf("getattr =>\t%d attr is cached, return cached data.\n", eid);
    goto RET;
  }
  ttprintf("getattr =>\t%d attr is not cached, call rpc.\n", eid);
  ret = cl->call(extent_protocol::getattr, eid, attr);
  ttprintf("getattr =>\t%d attr is not cached, call rpc done.\n", eid);
  //put cache
  cache.is_attr_cached = true;
  cache.attr_cache = attr;
  ttprintf("getattr =>\t%d attr is put into cache.\n", eid);
RET:
  pthread_mutex_unlock(&cache_map_mutex);
  return ret;
}


extent_protocol::status
extent_client_cache::get(extent_protocol::extentid_t eid, char* buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  std::string b;
  b = "";
  pthread_mutex_lock(&cache_map_mutex);
  Cache &cache = cache_map[eid];
  //check cache
  if (cache.is_data_cached) {
      memcpy(buf, cache.data_cache, BSIZE);
      ttprintf("get =>\t%d is cached, return cached data.\n", eid);
      goto RET;
  }
  ttprintf("get =>\t%d is not cached, call rpc.\n", eid);
  ret = cl->call(extent_protocol::get, eid, b);
  ttprintf("get =>\t%d is not cached, call rpc done.\n", eid);
  memcpy(buf, b.data(), BSIZE);
  //put cache
  cache.is_data_cached = true;
  cache.is_data_dirty = false;
  memcpy(cache.data_cache, buf, BSIZE);
  ttprintf("get =>\t%d is put into cache.\n", eid);
RET:
  pthread_mutex_unlock(&cache_map_mutex);
  return ret;
}


extent_protocol::status
extent_client_cache::put(extent_protocol::extentid_t eid, char* buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  pthread_mutex_lock(&cache_map_mutex);
  Cache &cache = cache_map[eid];
  //put cache
  cache.is_data_cached = true;
  cache.is_data_dirty = true;
  memcpy(cache.data_cache, buf, BSIZE);
  ttprintf("put =>\t%d put into cache.\n", eid);
  //std::string b(buf, BSIZE);
  //int a;
  //ret = cl->call(extent_protocol::put, eid, b, a);
  pthread_mutex_unlock(&cache_map_mutex);
  return ret;
}

extent_protocol::status
extent_client_cache::remove(extent_protocol::extentid_t eid)
{
  extent_protocol::status ret = extent_protocol::OK;
  pthread_mutex_lock(&cache_map_mutex);
  ttprintf("remove =>\tremove %d, rpc call.\n", eid);
  int a;
  ret = cl->call(extent_protocol::remove, eid, a);
  ttprintf("remove =>\tremove %d, rpc call done.\n", eid);
  ttprintf("remove =>\t%d removed from cache.\n", eid);
  cache_map.erase(eid);
  pthread_mutex_unlock(&cache_map_mutex);
  return ret;
}

extent_protocol::status
extent_client_cache::flush(extent_protocol::extentid_t eid)
{
  extent_protocol::status ret = extent_protocol::OK;
  int a;
  std::string b;
  pthread_mutex_lock(&cache_map_mutex);
  /*
  std::map<extent_protocol::extentid_t, Cache>::iterator it;
  for (it=cache_map.begin(); it!=cache_map.end(); ++it) {
    Cache &cache = it->second;
    //check dirty
    if (!cache.is_data_dirty) {
      ttprintf("flush =>\t%d is not dirty, return.\n", it->first);
      continue;
    }
    //put cache
    b = std::string(cache.data_cache, BSIZE);
    ttprintf("flush =>\t%d is dirty, rpc call.\n", it->first);
    ret = cl->call(extent_protocol::put, it->first, b, a);
    ttprintf("flush =>\t%d is dirty, rpc call done.\n", it->first);
  }
  ttprintf("flush => cache_map cleared.\n");
  cache_map.clear();
  */
  Cache &cache = cache_map[eid];
  if (!cache.is_data_dirty) {
      ttprintf("flush =>\t%d is not dirty, return.\n", eid);
      goto RET;
  }
  //put cache
  b = std::string(cache.data_cache, BSIZE);
  ttprintf("flush =>\t%d is dirty, rpc call.\n", eid);
  ret = cl->call(extent_protocol::put, eid, b, a);
  ttprintf("flush =>\t%d is dirty, rpc call done.\n", eid);
RET:
  //remove cache
  cache_map.erase(eid);
  ttprintf("flush =>\t%d removed from cache.\n", eid);
  //*/
  pthread_mutex_unlock(&cache_map_mutex);
  return ret;
}

extent_client_cache::~extent_client_cache() {
    //close(fd);
  pthread_mutex_destroy(&cache_map_mutex);
}

