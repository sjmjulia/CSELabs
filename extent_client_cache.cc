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

extent_client_cache::extent_client_cache(std::string dst)
{
  sockaddr_in dstsock;
  make_sockaddr(dst.c_str(), &dstsock);
  cl = new rpcc(dstsock);
  if (cl->bind() != 0) {
    printf("extent_client: bind failed\n");
  }
}
// a demo to show how to use RPC
extent_protocol::status
extent_client_cache::getattr(extent_protocol::extentid_t eid, 
                       extent_protocol::attr &attr)
{
  extent_protocol::status ret = extent_protocol::OK;
  ret = cl->call(extent_protocol::getattr, eid, attr);
  return ret;
}


extent_protocol::status
extent_client_cache::get(extent_protocol::extentid_t eid, char* buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  std::string b;
  b = "";
  ret = cl->call(extent_protocol::get, eid, b);
  memcpy(buf, b.data(), BSIZE);
  return ret;
}


extent_protocol::status
extent_client_cache::put(extent_protocol::extentid_t eid, char* buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  std::string b(buf, BSIZE);
  int a;
  ret = cl->call(extent_protocol::put, eid, b, a);
  return ret;
}

extent_protocol::status
extent_client_cache::remove(extent_protocol::extentid_t eid)
{
    extent_protocol::status ret = extent_protocol::OK;
    //fill this lab1
    return ret;
}


extent_client_cache::~extent_client_cache() {
    //close(fd);
}

