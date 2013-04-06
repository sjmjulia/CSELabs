// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sstream>
#include <map>
#include <string.h>


yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
  ec = new extent_client(extent_dst);

}

yfs_client::inum
yfs_client::n2i(std::string n)
{
  std::istringstream ist(n);
  unsigned long long finum;
  ist >> finum;
  return finum;
}

std::string
yfs_client::filename(inum inum)
{
  std::ostringstream ost;
  ost << inum;
  return ost.str();
}

bool
yfs_client::isfile(inum inum)
{
  //remove the following lines and fill this in lab1
  return false;
}

bool
yfs_client::isdir(inum inum)
{
  return ! isfile(inum);
}
int
yfs_client::getfile(inum inum, fileinfo &fin)
{
  //remove the following lines and fill this in lab1
  fin.atime = 0;
  fin.mtime = 0;
  fin.ctime = 0;
  fin.size = 0;
  return OK;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
  //remove the following lines and fill this in lab1
  din.atime = 0;
  din.mtime = 0;
  din.ctime = 0;
  return OK;
}


//implement functions:create,lookup,readdir,setattr,read,write,mkdir,unlink in lab1
int
yfs_client::create(inum parent, const std::string name,inum &ino)
{
  
  int r = IOERR;
  
 release:
  return r;
}

yfs_client::inum
yfs_client::lookup(inum parent, const std::string name)
{
  inum r = 0;
 release:
  return r;
}
int yfs_client::readdir(inum ino,std::map<inum,std::string> &items)
{
  int r = IOERR;
release:
  return r;
}

int yfs_client::setattr(inum ino,int to_set,const fileinfo &info)
{
  int r = IOERR;
  return r;
}
int yfs_client::read(inum ino,size_t size,off_t off,std::string &buf)
{
  int r = IOERR;
  release:
  return r;
}

int yfs_client::write(inum ino,const char* buf,size_t &size,off_t off)
{
  int r = IOERR;
  release:
  return r;
}

int
yfs_client::mkdir(inum parent, const std::string name,inum &ino)
{
  int r = IOERR;
 release:
  return r;
}

int
yfs_client::unlink(inum parent, const std::string name)
{
  int r = IOERR;
 release:
  return r;
}
