// the extent server implementation

#include "extent_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ttprintf.h"

extent_server::extent_server() 
{
    umask(0000);
    FILE *f;
    while (1) {
        f = fopen("disk", "wb+");
        if (f) break;
    }
    fclose(f);
}


int extent_server::put(extent_protocol::extentid_t id, std::string buf, int &)
{
  // You fill this in for Lab 2.
  
    ttprintf("put =>\t%lld\n", id);
    int ret = extent_protocol::OK;
    int fd = 0;
    while (-1 == (fd = open("disk", O_RDWR | O_CREAT)));
    size_t cnt = 0;
    lseek(fd, id * BSIZE, SEEK_SET);

    if (BSIZE != (cnt = write(fd, buf.data(), BSIZE))) {
        ret = extent_protocol::IOERR;
    }
    if (0 == cnt) {
        ret = extent_protocol::NOENT;
    }

    close(fd);
    return ret;
  return extent_protocol::IOERR;
}

int extent_server::get(extent_protocol::extentid_t id, std::string &buf)
{
  // You fill this in for Lab 2.
    ttprintf("get ===>\t%lld\n", id);
    char b[BSIZE];
    int ret = extent_protocol::OK;
    int fd = 0;
    while (-1 == (fd = open("disk", O_RDWR | O_CREAT)));
    size_t cnt = 0;
    while (BSIZE != cnt) {
        lseek(fd, id * BSIZE, SEEK_SET);
        cnt = read(fd, b, BSIZE);
    }
    buf = std::string(b, BSIZE);
    close(fd);
    return ret;
  return extent_protocol::IOERR;
}

// a demo to show how to implement server side
int extent_server::getattr(extent_protocol::extentid_t id, extent_protocol::attr &a)
{
  a.size = 0;
  a.atime = 0;
  a.mtime = 0;
  a.ctime = 0;
  return extent_protocol::OK;
}

int extent_server::remove(extent_protocol::extentid_t id, int &)
{
  // You fill this in for Lab 2.
  return extent_protocol::OK;
  return extent_protocol::IOERR;
}

