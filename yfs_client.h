#ifndef yfs_client_h
#define yfs_client_h

#include <string>
//#include "yfs_protocol.h"
#include "extent_client.h"
#include <vector>
#include <map>
#include <fcntl.h>
#include "lock_client.h"
#include "lock_client_cache.h"

#define YFS_SET_ATTR_SIZE  0x1
class yfs_client {
  extent_client *ec;
  lock_client *lc;
  class lock_flush_cache : public lock_release_user {
   private:
    extent_client *holy_shit_ec;
   public:
    lock_flush_cache(extent_client *);
    virtual void dorelease(lock_protocol::lockid_t);
  };

 public:

  typedef unsigned long long inum;
  enum xxstatus { OK, RPCERR, NOENT, IOERR, EXIST };
  typedef int status;

  struct fileinfo {
    unsigned long long size;
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirinfo {
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirent {
    std::string name;
    yfs_client::inum inum;
  };

 private:
  static std::string filename(inum);
  static inum n2i(std::string);

    //filling begin
    int safe_get(inum ino, char *buf, inum ignore);
    int safe_put(inum &ino, char *buf, bool add, inum ignore);

    int get(inum ino, char *buf);
    int put(inum &ino, char *buf, bool add);
    int remove(inum ino);
    int recursive_unlink(inum ino);
    //filling end

 public:
  yfs_client(std::string extent_dst, std::string lock_dst);
  bool isfile(inum);
  bool isdir(inum);
  int getfile(inum, fileinfo &);
  int getdir(inum, dirinfo &);
  int create(inum parent, const std::string name,inum &ino);
  int readdir(inum ino,std::map<inum,std::string> &items);
  int setattr(inum ino,int to_set,const fileinfo &info);
  int read(inum ino,size_t size,off_t off,std::string &buf);
  int write(inum ino,const char* buf,size_t &size,off_t off);

  inum lookup(inum parent, const std::string name);
  int mkdir(inum parent, const std::string name,inum &ino);
  int unlink(inum parent, const std::string name);

  //finding begin
  int inner_create(inum parent, const std::string name,inum &ino);
  int inner_read(inum ino,size_t size,off_t off,std::string &buf);
  int inner_write(inum ino,const char* buf,size_t &size,off_t off);
  bool issym(inum ino);
  int symlink(inum parent, const std::string name, const std::string link, inum &ino);
  int readlink(inum ino, std::string &link);
  //finding end

};

#define INODE_DIRECT_BLOCK_NUM 16
#define INODE_DIR_ENTRY_NUM 5
#define INODE_FILE_NAME_LEN 64
#define SUPER_BLOCK_IDENTITY 0
#define SUPER_BLOCK_INODE_NUM_OFFSET 32
#define SUPER_BLOCK_ROOT_INODE_OFFSET 64
#define SUPER_BLOCK_BLOCK_BITMAP_OFFSET 80
#define BLOCK_BITMAP_BEGIN_BLOCK 10
#define BLOCK_BITMAP_END_BLOCK 200
#define BITMAP_PER_BLOCK 4096

struct inode
{
  //fill this in lab1
    //filling begin
    char mode;
    unsigned long long size;
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
    union {//info{
        struct {//file_node {
            yfs_client::inum direct_blocks[INODE_DIRECT_BLOCK_NUM];
        };
        struct {// dir_node {
            struct {
                char name[INODE_FILE_NAME_LEN];
                yfs_client::inum inum;
            }dir_entries[INODE_DIR_ENTRY_NUM];
        };
    };
    yfs_client::inum next_inode;
    //filling end
};
#define M_INODE_PTR(x) ((inode *)(x))

#endif 
