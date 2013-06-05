// RPC stubs for clients to talk to extent_server

#include "extent_client.h"
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

extent_client::extent_client(std::string dst)
{
  sockaddr_in dstsock;
  make_sockaddr(dst.c_str(), &dstsock);
  cl = new rpcc(dstsock);
  if (cl->bind() != 0) {
    printf("extent_client: bind failed\n");
  }
    /*
     * written in lab1
    umask(0000);
    FILE *f;
    while (1) {
        f = fopen("disk", "wb+");
        if (f) break;
    }
    fclose(f);
    */
}
// a demo to show how to use RPC
extent_protocol::status
extent_client::getattr(extent_protocol::extentid_t eid, 
                       extent_protocol::attr &attr)
{
  extent_protocol::status ret = extent_protocol::OK;
  ret = cl->call(extent_protocol::getattr, eid, attr);
  return ret;
}


extent_protocol::status
extent_client::get(extent_protocol::extentid_t eid, char* buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  std::string b;
  b = "";
  ret = cl->call(extent_protocol::get, eid, b);
  memcpy(buf, b.data(), BSIZE);
  return ret;
    /*
     * written in lab1
    extent_protocol::status ret = extent_protocol::OK;
    while (-1 == (fd = open("disk", O_RDWR | O_CREAT)));
    size_t cnt = 0;
    while (BSIZE != cnt) {
        lseek(fd, eid * BSIZE, SEEK_SET);
        cnt = read(fd, buf, BSIZE);
    }
    close(fd);
    return ret;
    */
}


extent_protocol::status
extent_client::put(extent_protocol::extentid_t eid, char* buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  std::string b(buf, BSIZE);
  int a;
  ret = cl->call(extent_protocol::put, eid, b, a);
  return ret;
    /*
     * written in lab1
    extent_protocol::status ret = extent_protocol::OK;
    while (-1 == (fd = open("disk", O_RDWR | O_CREAT)));
    size_t cnt = 0;
    lseek(fd, eid * BSIZE, SEEK_SET);

    if (BSIZE != (cnt = write(fd, buf, BSIZE))) {
        ret = extent_protocol::IOERR;
    }
    if (0 == cnt) {
        ret = extent_protocol::NOENT;
    }
    close(fd);
    return ret;
    */
}

extent_protocol::status
extent_client::remove(extent_protocol::extentid_t eid)
{
    extent_protocol::status ret = extent_protocol::OK;
    //fill this lab1
    return ret;
}


extent_client::~extent_client() {
    //close(fd);
}

extent_protocol::status
extent_client::flush(extent_protocol::extentid_t eid)
{
    extent_protocol::status ret = extent_protocol::OK;
    return ret;
}

