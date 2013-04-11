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
    //filling begin
    umask(0000);
    FILE *f;
    while (1) {
        //fd = open("disk", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG);
        f = fopen("disk", "wb+");
        if (f) break;
    }
    //close(fd);
    fclose(f);
    //filling end
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
    //fill this for lab1
    //filling begin
    while (-1 == (fd = open("disk", O_RDWR | O_CREAT)));
    size_t cnt = 0;
    //lseek(fd, eid * BSIZE, SEEK_SET);

    /*
    if (BSIZE != (cnt = read(fd, buf, BSIZE))) {
        ret = extent_protocol::IOERR;
    }
    */
    while (BSIZE != cnt) {
        lseek(fd, eid * BSIZE, SEEK_SET);
        cnt = read(fd, buf, BSIZE);
    }

    close(fd);
    //filling end
    return ret;
}


extent_protocol::status
extent_client::put(extent_protocol::extentid_t eid, char* buf)
{
    extent_protocol::status ret = extent_protocol::OK;
    //fill this for lab1
    //filling begin
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
    //filling end
    return ret;
}

extent_protocol::status
extent_client::remove(extent_protocol::extentid_t eid)
{
    extent_protocol::status ret = extent_protocol::OK;
    //fill this lab1
    return ret;
}


//filling begin
extent_client::~extent_client() {
    //close(fd);
}
//filling end
