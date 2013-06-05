// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
//#include "extent_client.h"
#include "extent_client_cache.h"
#include "lock_client_cache.h"
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
#include "ttprintf.h"

yfs_client::lock_flush_cache::lock_flush_cache(extent_client *hsec)
  :holy_shit_ec(hsec)
{
    //nothing to do
    ttprintf("lock_flush_cache => I am living to death...\n");
}

void
yfs_client::lock_flush_cache::dorelease(lock_protocol::lockid_t lid)
{
    ttprintf("lock_flush_cache::dorelease=> flusing lid %d...\n", lid);
  holy_shit_ec->flush(lid);
}

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
#if 0
  ec = new extent_client(extent_dst);
#else
  ec = new extent_client_cache(extent_dst);
#endif
#if 0
  lc = new lock_client(lock_dst);
#else
  lc = new lock_client_cache(lock_dst, new lock_flush_cache(ec));
#endif
    //filling begin
    char buf[BSIZE];
    memset(buf, 0, BSIZE);
    //identity
    strcpy(buf, "This is super block!");
    //inode-block num
    *(inum *)(buf + SUPER_BLOCK_INODE_NUM_OFFSET) = 1024;
    *(inum *)(buf + SUPER_BLOCK_ROOT_INODE_OFFSET) = 0;
    //block-bitmap
    *(buf + SUPER_BLOCK_BLOCK_BITMAP_OFFSET) = 1;
    *(buf + SUPER_BLOCK_BLOCK_BITMAP_OFFSET + (1 >> 3)) |= 1 << (1 & 0x7);
    lc->acquire(0);
    ec->put(0, buf);
    lc->release(0);
    /////////////////////BLOCK-BITMAP from 10 ~ 200
    memset(buf, 0, BSIZE);
    lc->acquire(BLOCK_BITMAP_END_BLOCK);
    ec->put(BLOCK_BITMAP_END_BLOCK, buf);
    lc->release(BLOCK_BITMAP_END_BLOCK);
    *(buf) = 1;
    *(buf + (1 >> 3)) |= 1 << (1 & 0x7);
    for (int i=BLOCK_BITMAP_BEGIN_BLOCK; i<=BLOCK_BITMAP_END_BLOCK; ++i) {
        *(buf + (i >> 3)) |= 1 << (i & 0x7);
    }
    lc->acquire(BLOCK_BITMAP_BEGIN_BLOCK);
    ec->put(BLOCK_BITMAP_BEGIN_BLOCK, buf);
    lc->release(BLOCK_BITMAP_BEGIN_BLOCK);

    // root block
    memset(buf, 0, BSIZE);
    ((inode*)buf)->mode = 0;
    ((inode*)buf)->size = 0;
    ((inode*)buf)->atime = 0;
    ((inode*)buf)->mtime = 0;
    ((inode*)buf)->ctime = 0;
    lc->acquire(1);
    ec->put(1, buf);
    lc->release(1);
    //filling end
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
  //return false;
    //filling begin
    char buf[BSIZE];
    bool ret = false;
    lc->acquire(inum);
    ttprintf("isfile:=> locked %lld\n", inum);
    if (OK != safe_get(inum, buf, inum)) {
        ret = false;
        goto over;
    }
    ret = (1==M_INODE_PTR(buf)->mode);
    //filling end
over:
    lc->release(inum);
    ttprintf("isfile:=> unlocked %lld\n", inum);
    return ret;
}

bool
yfs_client::isdir(inum inum)
{
    //filling begin
    char buf[BSIZE];
    bool ret = false;
    ttprintf("isdir:=> locked %lld\n", inum);
    lc->acquire(inum);
    if (OK != safe_get(inum, buf, inum)) {
        ret = false;
        goto over;
    }
    ret = (0==M_INODE_PTR(buf)->mode);
over:
    lc->release(inum);
    ttprintf("isdir:=> unlocked %lld\n", inum);
    return ret;
    //filling begin
  return ! isfile(inum);
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
  //remove the following lines and fill this in lab1
  //fin.atime = 0;
  //fin.mtime = 0;
  //fin.ctime = 0;
  //fin.size = 0;
    //filling begin
    char buf[BSIZE];
    ttprintf("getfile:=> locked %lld\n", inum);
    lc->acquire(inum);
    int ret = safe_get(inum, buf, inum);
    if (NOENT == ret) {
        ret = NOENT;
        goto over;
    }
    if (OK != ret) {
        ret = IOERR;
        goto over;
    }
    fin.atime = M_INODE_PTR(buf)->atime;
    fin.mtime = M_INODE_PTR(buf)->mtime;
    fin.ctime = M_INODE_PTR(buf)->ctime;
    fin.size = M_INODE_PTR(buf)->size;
    //filling end
    ret = OK;
over:
    lc->release(inum);
    ttprintf("getfile:=> unlocked %lld\n", inum);
    return ret;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
  //remove the following lines and fill this in lab1
  //din.atime = 0;
  //din.mtime = 0;
  //din.ctime = 0;
    //filling begin
    char buf[BSIZE];
    lc->acquire(inum);
    ttprintf("getdir:=> locked %lld\n", inum);
    int ret = safe_get(inum, buf, inum);
    if (NOENT == ret) {
        goto over;
    }
    if (OK != ret) {
        ret = IOERR;
        goto over;
    }
    din.atime = M_INODE_PTR(buf)->atime;
    din.mtime = M_INODE_PTR(buf)->mtime;
    din.ctime = M_INODE_PTR(buf)->ctime;
    //filling end
    ret = OK;
over:
    lc->release(inum);
    ttprintf("getdir:=> unlocked %lld\n", inum);
    return ret;
}


//filling begin
int yfs_client::get(inum ino, char *buf) {
    if (ino < BLOCK_BITMAP_BEGIN_BLOCK || ino > BLOCK_BITMAP_END_BLOCK) {

        inum bitmap_block = BLOCK_BITMAP_BEGIN_BLOCK + ino / BITMAP_PER_BLOCK;
        inum bitmap = ino % BITMAP_PER_BLOCK;
        char bitmap_block_buf[BSIZE];
        //safe_get(bitmap_block, bitmap_block_buf, 0);
        lc->acquire(bitmap_block);
        ec->get(bitmap_block, bitmap_block_buf);
        lc->release(bitmap_block);
        if (!((*(bitmap_block_buf + (bitmap >> 3)) >> (bitmap & 0x7)) & 1)) {
            return NOENT;
        }
    }
    return ec->get(ino, buf);
}

int yfs_client::put(inum &ino, char *buf, bool add) {
    if (add) {
        int ret = OK;
        char bitmap_block_buf[BSIZE];
        inum last_bitmap_block = -1;
        for (inum i=0; ; ++i) {
            inum bitmap_block = BLOCK_BITMAP_BEGIN_BLOCK + i / BITMAP_PER_BLOCK;
            if (bitmap_block != last_bitmap_block) {
                lc->acquire(bitmap_block);
                last_bitmap_block = bitmap_block;
            }
            inum bitmap = i % BITMAP_PER_BLOCK;
            //safe_get(bitmap_block, bitmap_block_buf, bitmap_block);
            ec->get(bitmap_block, bitmap_block_buf);
            if (!((*(bitmap_block_buf + (bitmap >> 3)) >> (bitmap & 0x7)) & 1)) {
                lc->acquire(i);
                ret = ec->put(i, buf);
                lc->release(i);
                if (OK != ret) {
                    ret = IOERR;
                    goto over;
                }
                *(bitmap_block_buf + (bitmap >> 3)) |= 1 << (bitmap & 0x7);
                if (OK != ec->put(bitmap_block, bitmap_block_buf)) {
                    ret = IOERR;
                    goto over;
                }
                ino = i;
                ret = OK;
                goto over;
            }
        }
        ret = IOERR;
over:
        while (last_bitmap_block >= BLOCK_BITMAP_BEGIN_BLOCK) {
            lc->release(last_bitmap_block);
            --last_bitmap_block;
        }
        return ret;

    } else {
        if (ino < BLOCK_BITMAP_BEGIN_BLOCK || ino > BLOCK_BITMAP_END_BLOCK) {
            inum bitmap_block = BLOCK_BITMAP_BEGIN_BLOCK + ino / BITMAP_PER_BLOCK;
            inum bitmap = ino % BITMAP_PER_BLOCK;
            char bitmap_block_buf[BSIZE];
            //safe_get(bitmap_block, bitmap_block_buf, 0);
            
            lc->acquire(bitmap_block);
            ec->get(bitmap_block, bitmap_block_buf);
            lc->release(bitmap_block);

            if (!((*(bitmap_block_buf + (bitmap >> 3)) >> (bitmap & 0x7)) & 1)) {
                return NOENT;
            }
        }
        ec->put(ino, buf);
        return OK;
    }
}

int yfs_client::remove(inum ino) {
    inum bitmap_block = BLOCK_BITMAP_BEGIN_BLOCK + ino / BITMAP_PER_BLOCK;
    inum bitmap = ino % BITMAP_PER_BLOCK;
    char bitmap_block_buf[BSIZE];
    //safe_get(bitmap_block, bitmap_block_buf, bitmap_block);
    lc->acquire(bitmap_block);

    ec->get(bitmap_block, bitmap_block_buf);
    *(bitmap_block_buf + (bitmap >> 3)) &= ~(1 << (bitmap & 0x7));
    int ret = IOERR;
    //if (OK != safe_put(bitmap_block, bitmap_block_buf, 0, bitmap_block)) {
    if (OK != ec->put(bitmap_block, bitmap_block_buf)) {
       ret = IOERR;
       goto over;
    }
    ret = OK;
over:
    lc->release(bitmap_block);
    return ret;
}
//filling end

//implement functions:create,lookup,readdir,setattr,read,write,mkdir,unlink in lab1
int
yfs_client::create(inum parent, const std::string name,inum &ino)
{
  
    ttprintf("create===================> parent: %lld name: %s ino: %lld\n", parent, name.c_str(), ino);
    int ret = OK;
    char buf[BSIZE];
    char tmp_buf[BSIZE];
    inum tmp = 0;
    inum target = parent;

    lc->acquire(parent);                                            //+
    ttprintf("create:=> locked %lld\n", parent);
    safe_get(parent, buf, parent);
    M_INODE_PTR(buf)->ctime = M_INODE_PTR(buf)->mtime = time(0);
    if (OK != safe_put(parent, buf, 0, parent)) {
        ret = IOERR;
        goto over;
    }

    //search new blank entry
    while (M_INODE_PTR(buf)->size >= INODE_DIR_ENTRY_NUM) {
        //check duplicated
        for (inum i=0; i<M_INODE_PTR(buf)->size; ++i) {
            if (!strcmp(M_INODE_PTR(buf)->dir_entries[i].name, name.c_str())) {
                ino = (M_INODE_PTR(buf)->dir_entries[i].inum);
                goto over;
            } else {
                ttprintf("create:=> found key %lld value %s\n",
                        M_INODE_PTR(buf)->dir_entries[i].inum,
                        M_INODE_PTR(buf)->dir_entries[i].name);

            }
        }
        //proess
        if (M_INODE_PTR(buf)->next_inode) {
            target = M_INODE_PTR(buf)->next_inode;
            safe_get(target, buf, parent);
        } else {
            memset(tmp_buf, 0, BSIZE);
            if (OK != safe_put(tmp, tmp_buf, 1, parent)) {
                ret = IOERR;
                goto over;
            }
            M_INODE_PTR(buf)->next_inode = tmp;
            if (OK != safe_put(target, buf, 0, parent)) {
                ret = IOERR;
                goto over;
            }
        }
    }
    //check duplicated
    for (inum i=0; i<M_INODE_PTR(buf)->size; ++i) {
        if (!strcmp(M_INODE_PTR(buf)->dir_entries[i].name, name.c_str())) {
            ino = (M_INODE_PTR(buf)->dir_entries[i].inum);
            goto over;
        } else {
            ttprintf("create:=> found key %lld value %s\n",
                    M_INODE_PTR(buf)->dir_entries[i].inum,
                    M_INODE_PTR(buf)->dir_entries[i].name);
        }
    }
    //new file block
    memset(tmp_buf, 0, BSIZE);
    M_INODE_PTR(tmp_buf)->mode = 1;
    M_INODE_PTR(tmp_buf)->atime = 0;
    M_INODE_PTR(tmp_buf)->mtime = time(0);
    M_INODE_PTR(tmp_buf)->ctime = time(0);
    if (OK != safe_put(tmp, tmp_buf, 1, parent)) {
        ret = IOERR;
        goto over;
    }
    //fill in entry
    strcpy(M_INODE_PTR(buf)->dir_entries[M_INODE_PTR(buf)->size].name, name.c_str());
    M_INODE_PTR(buf)->dir_entries[M_INODE_PTR(buf)->size].inum = tmp;
    M_INODE_PTR(buf)->size++;
    if (OK != safe_put(target, buf, 0, parent)) {
        ret = IOERR;
        goto over;
    }
    ino = tmp;
    ret = OK;

over:
    /*
    safe_get(parent, buf, parent);
    while (1){
        if (!M_INODE_PTR(buf)->next_inode) break;
        safe_get(M_INODE_PTR(buf)->next_inode, buf, parent);
    }
    */
    lc->release(parent);                                    //-
    ttprintf("create:=> unlocked %lld\n", parent);
    /*

    std::map<inum,std::string> items;
    std::map<inum,std::string>::iterator it;
    readdir(parent, items);
    int count = 0;
    for (it=items.begin(); it!=items.end(); ++it) {
        ttprintf("%d: inode %lld name %s\n", count++, it->first, it->second.c_str());
    }
    */


    return ret;
}

yfs_client::inum
yfs_client::lookup(inum parent, const std::string name)
{
    inum ret = 0;
    char buf[BSIZE];

    lc->acquire(parent);                                    //+
    ttprintf("lookup:=> locked %lld\n", parent);

    safe_get(parent, buf, parent);
    M_INODE_PTR(buf)->atime = time(0);
    if (OK != safe_put(parent, buf, 0, parent)) {
        ////////trick here.
        ret = IOERR;
        goto over;
    }
    //search new blank entry
    while (1){
        for (inum i=0; i<M_INODE_PTR(buf)->size; ++i) {
            if (!strcmp(M_INODE_PTR(buf)->dir_entries[i].name, name.c_str())) {
                ret = (M_INODE_PTR(buf)->dir_entries[i].inum);
                goto over;
            }
        }
        if (!M_INODE_PTR(buf)->next_inode) break;
        safe_get(M_INODE_PTR(buf)->next_inode, buf, parent);
    }

over:
    lc->release(parent);
    ttprintf("lookup:=> unlocked %lld\n", parent);
    return ret;
}

int yfs_client::readdir(inum ino,std::map<inum,std::string> &items)
{
    int ret = IOERR;
    char buf[BSIZE];

    lc->acquire(ino);                                       //+
    ttprintf("readdir:=> locked %lld\n", ino);
    items.clear();
    safe_get(ino, buf, ino);
    M_INODE_PTR(buf)->atime = time(0);
    if (OK != safe_put(ino, buf, 0, ino)) {
        ret = IOERR;
        goto over;
    }
    //search new blank entry
    while (1){
        for (inum i=0; i<M_INODE_PTR(buf)->size; ++i) {
            items[M_INODE_PTR(buf)->dir_entries[i].inum]
                = std::string(M_INODE_PTR(buf)->dir_entries[i].name);
            ttprintf("readdir:=> key %lld value %s\n", 
                    M_INODE_PTR(buf)->dir_entries[i].inum, 
                    M_INODE_PTR(buf)->dir_entries[i].name);
        }
        if (!M_INODE_PTR(buf)->next_inode) break;
        safe_get(M_INODE_PTR(buf)->next_inode, buf, ino);
    }
    ret = OK;
over:
    lc->release(ino);                                       //-
    ttprintf("readdir:=> unlocked %lld\n", ino);
    return ret;
}

int yfs_client::setattr(inum ino,int to_set,const fileinfo &info)
{
    int ret = OK;
    char buf[BSIZE];
    lc->acquire(ino);                                       //+
    ttprintf("setattr:=> locked %lld\n", ino);
    safe_get(ino, buf, ino);
    M_INODE_PTR(buf)->atime = info.atime;
    M_INODE_PTR(buf)->mtime = info.mtime;
    M_INODE_PTR(buf)->ctime = info.ctime;
    if (to_set) {
        inum tmp = ino;
        size_t write_size = 0;
        inner_write(tmp, (const char *)buf, write_size, (off_t)info.size);
        M_INODE_PTR(buf)->size = info.size;
    }
    if (OK != safe_put(ino, buf, 0, ino)) {
        ret = IOERR;
        goto over;
    }
over:
    lc->release(ino);                                       //-
    ttprintf("setattr:=> unlocked %lld\n", ino);
    return ret;
}

int yfs_client::read(inum ino,size_t size,off_t off,std::string &buf)
{
    lc->acquire(ino);
    int ret = inner_read(ino, size, off, buf);
    lc->release(ino);
    return ret;
}

int yfs_client::write(inum ino,const char* buf,size_t &size,off_t off)
{
    lc->acquire(ino);
    int ret = inner_write(ino, buf, size, off);
    lc->release(ino);
    return ret;
}

int
yfs_client::mkdir(inum parent, const std::string name,inum &ino)
{
    char buf[BSIZE];
    char tmp_buf[BSIZE];

    lc->acquire(parent);

    inum tmp = 0;
    safe_get(parent, buf, parent);
    M_INODE_PTR(buf)->ctime = time(0);
    M_INODE_PTR(buf)->mtime = time(0);
    safe_put(parent, buf, 0, parent);

    inum target = parent;
    //search new blank entry
    while (M_INODE_PTR(buf)->size >= INODE_DIR_ENTRY_NUM) {
        /////////////////check duplicate ?
        if (M_INODE_PTR(buf)->next_inode) {
            target = M_INODE_PTR(buf)->next_inode;
            safe_get(target, buf, parent);
        } else {
            memset(tmp_buf, 0, BSIZE);
            safe_put(tmp, tmp_buf, 1, parent);
            M_INODE_PTR(buf)->next_inode = tmp;
            safe_put(target, buf, 0, parent);
        }
    }
    //new file block
    memset(tmp_buf, 0, BSIZE);
    M_INODE_PTR(tmp_buf)->atime = 0;
    M_INODE_PTR(tmp_buf)->mtime = time(0);
    M_INODE_PTR(tmp_buf)->ctime = time(0);
    safe_put(tmp, tmp_buf, 1, parent);
    //fill in entry
    strcpy(M_INODE_PTR(buf)->dir_entries[M_INODE_PTR(buf)->size].name, name.c_str());
    M_INODE_PTR(buf)->dir_entries[M_INODE_PTR(buf)->size].inum = tmp;
    M_INODE_PTR(buf)->size++;
    safe_put(target, buf, 0, parent);
    ino = tmp;

    lc->release(parent);
    return OK;
}

int
yfs_client::unlink(inum parent, const std::string name)
{
    ttprintf("unlink===================> parent: %lld name: %s\n", parent, name.c_str());
    int ret = OK;

    char buf[BSIZE];
    inum unlink_inum = 0;
    inum buf_inum = parent;
    
    lc->acquire(parent);

    safe_get(parent, buf, parent);
    M_INODE_PTR(buf)->mtime = time(0);
    M_INODE_PTR(buf)->ctime = time(0);
    safe_put(parent, buf, 0, parent);
    inum index = 0;
    bool found = false;
    while (1){
        for (index=0; index<M_INODE_PTR(buf)->size; ++index) {
            if (!strcmp(M_INODE_PTR(buf)->dir_entries[index].name, name.c_str())) {
                found = true;
                break;
            }
        }
        if (found) break;
        if (!M_INODE_PTR(buf)->next_inode) break;
        buf_inum = M_INODE_PTR(buf)->next_inode;
        safe_get(buf_inum, buf, parent);
    }
    if (!found) {
        ttprintf("unlink: not found!\n");
        ret = NOENT;
        goto over;
    }
    unlink_inum = M_INODE_PTR(buf)->dir_entries[index].inum;
    char new_buf[BSIZE];
    memset(new_buf, 0, BSIZE);
    M_INODE_PTR(new_buf)->mode = M_INODE_PTR(buf)->mode;
    M_INODE_PTR(new_buf)->atime = M_INODE_PTR(buf)->atime;
    M_INODE_PTR(new_buf)->mtime = M_INODE_PTR(buf)->mtime;
    M_INODE_PTR(new_buf)->ctime = M_INODE_PTR(buf)->ctime;
    M_INODE_PTR(new_buf)->next_inode = M_INODE_PTR(buf)->next_inode;
    M_INODE_PTR(new_buf)->size = 0;
    for (inum i=0; i<M_INODE_PTR(buf)->size; ++i) {
        if (i == index) continue;
        M_INODE_PTR(new_buf)->dir_entries[M_INODE_PTR(new_buf)->size]
            = M_INODE_PTR(buf)->dir_entries[i];
        M_INODE_PTR(new_buf)->size++;
    }
    safe_put(buf_inum, new_buf, 0, parent);

    recursive_unlink(unlink_inum);
over:
    lc->release(parent);
    ttprintf("sfsdddddddddddddddddddddddddddddddddddddddddddddddddddddddd %d\n", ret);
    /*
    std::map<inum,std::string> items;
    std::map<inum,std::string>::iterator it;
    readdir(parent, items);
    int count = 0;
    for (it=items.begin(); it!=items.end(); ++it) {
        ttprintf("%d: inode %lld name %s\n", count++, it->first, it->second.c_str());
    }
    */
    return ret;
}

//filling begin
int yfs_client::recursive_unlink(inum ino) {
    if (!ino) return 0;
    char buf[BSIZE];
    safe_get(ino, buf, 0);
    for (inum i=0; i<INODE_DIRECT_BLOCK_NUM; ++i) {
        if (M_INODE_PTR(buf)->direct_blocks[i]) {
            remove(M_INODE_PTR(buf)->direct_blocks[i]);
        } else break;
    }
    if (M_INODE_PTR(buf)->next_inode) {
        recursive_unlink(M_INODE_PTR(buf)->next_inode);
        //remove(M_INODE_PTR(buf)->next_inode);
    }
    remove(ino);
    return 0;
}
//filling end

//finding begin
bool
yfs_client::issym(inum ino)
{
    char buf[BSIZE];
    lc->acquire(ino);
    ttprintf("issym:=> %lld\n", ino);
    if (OK != get(ino, buf)) {
        return false;
    }
    lc->release(ino);
    return 2==M_INODE_PTR(buf)->mode;
}

int
yfs_client::symlink(inum parent, const std::string name, 
        const std::string link, inum &ino)
{
    std::cout << "sdf sd " << std::endl;
    inum tmp;
    lc->acquire(parent);
    ttprintf("symlink:=> locked %lld\n", parent);
    int ret = inner_create(parent, name, tmp);
    std::cout << "createdsd " << std::endl;
    size_t len = link.size();

    if (OK != ret) goto ret_pnt;

    ret = inner_write(tmp, link.c_str(), len, (off_t)0);
    std::cout << "writedcr" << std::endl;

    if (OK != ret) goto ret_pnt;;

    char buf[BSIZE];
    if (OK != safe_get(tmp, buf, parent)) goto ret_pnt;

    std::cout << "moded" << std::endl;
    M_INODE_PTR(buf)->mode = 2;
    if (OK != safe_put(tmp, buf, 0, parent)) goto ret_pnt;

    ino = tmp;
ret_pnt:
    lc->release(parent);
    ttprintf("symlink:=> unlocked %lld\n", parent);
    return ret;
}

int
yfs_client::readlink(inum ino, std::string &link)
{
    
    int ret = 0;
    fileinfo info;

    //getfile(ino, info);
    char buf[BSIZE];
    ttprintf("readlink:=> locked %lld\n", ino);
    lc->acquire(ino);
    ret = safe_get(ino, buf, ino);
    if (NOENT == ret) {
        ret = NOENT;
        goto over;
    }
    if (OK != ret) {
        ret = IOERR;
        goto over;
    }
    info.atime = M_INODE_PTR(buf)->atime;
    info.mtime = M_INODE_PTR(buf)->mtime;
    info.ctime = M_INODE_PTR(buf)->ctime;
    info.size = M_INODE_PTR(buf)->size;
    //filling end
    ret = OK;
    ret = inner_read(ino, info.size, 0, link);

over:
    lc->release(ino);
    ttprintf("readlink:=> unlocked %lld\n", ino);
    return ret;

}

int yfs_client::inner_read(inum ino,size_t size,off_t off,std::string &buf)
{
    int r = IOERR;
    char inode_buf[BSIZE];
    char data_buf[BSIZE];
    buf = "";
    if (0 == size) return OK;

    if (NOENT == safe_get(ino, inode_buf, ino)) {
        return NOENT;
    }
    M_INODE_PTR(inode_buf)->atime = time(0);
    if (OK != safe_put(ino, inode_buf, 0, ino)) {
        return IOERR;
    }
    inum filesize = M_INODE_PTR(inode_buf)->size;
    inum index = 0;
    //offsetting
    while (1) {
        //touch the data block
        if (index >= INODE_DIRECT_BLOCK_NUM) {
            if (M_INODE_PTR(inode_buf)->next_inode) {
                safe_get(M_INODE_PTR(inode_buf)->next_inode, inode_buf, ino);
            } else {
                //return EOF;
                return OK;
            }
            index = 0;
        }
        if (!M_INODE_PTR(inode_buf)->direct_blocks[index]) {
            //return EOF;
            return OK;
        }

        //calculate offset
        if (off >= BSIZE) {
            if (filesize >= BSIZE) {
                off -= BSIZE;
                filesize -= BSIZE;
                ++index;
            } else {
                //return EOF;
                return OK;
            }
        } else {
            break;
        }
    }
    //reading
    while (1) {
        //touch the data block
        if (index >= INODE_DIRECT_BLOCK_NUM) {
            if (M_INODE_PTR(inode_buf)->next_inode) {
                safe_get(M_INODE_PTR(inode_buf)->next_inode, inode_buf, ino);
            } else {
                //EOF;
                return OK;
            }
            index = 0;
        }
        if (!M_INODE_PTR(inode_buf)->direct_blocks[index]) {
            //return EOF;
            return OK;
        }

        //reading the data
        if (size <= filesize) {
            if (size <= BSIZE - off) {
                safe_get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf, ino);
                //may be a \0 in data
                buf.append(data_buf + off, size);
                filesize -= size;
                size = 0;
                break;
            } else {
                safe_get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf, ino);
                buf.append(data_buf + off, BSIZE - off);
                size -= BSIZE - off;
                filesize -= BSIZE - off;
                off = 0;
                ++index;
            }
        } else {
            //size > filesize
            r = EOF;
            if (filesize <= BSIZE - off) {
                safe_get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf, ino);
                //may be a \0 in data
                buf.append(data_buf + off, filesize);
                size -= filesize;
                filesize = 0;
                break;
            } else {
                safe_get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf, ino);
                buf.append(data_buf + off, BSIZE - off);
                size -= BSIZE - off;
                filesize -= BSIZE - off;
                off = 0;
                ++index;
            }
        }
    }
    return OK;
    return r;
}

int yfs_client::inner_write(inum ino,const char* buf,size_t &size,off_t off)
{
  int r = IOERR;
    char inode_buf[BSIZE];
    char tmp_buf[BSIZE];
    char data_buf[BSIZE];
    size_t saved_size = size;
    memset(tmp_buf, 0, BSIZE);
    inum tmp = 0;

    if (NOENT == safe_get(ino, inode_buf, ino)) {
        return NOENT;
    }
    M_INODE_PTR(inode_buf)->atime = time(0);
    M_INODE_PTR(inode_buf)->mtime = time(0);
    if (OK != safe_put(ino, inode_buf, 0, ino)) {
        return IOERR;
    }
    long long filesize = M_INODE_PTR(inode_buf)->size;
    inum new_filesize = filesize > off + size ? filesize : off + size;
    inum target = ino;
    inum index = 0;
    //offsetting
    while (1) {
        //touch the data block
        if (index >= INODE_DIRECT_BLOCK_NUM) {
            if (!M_INODE_PTR(inode_buf)->next_inode) {
                //create next inode
                tmp = 0;
                memset(tmp_buf, 0, BSIZE);
                M_INODE_PTR(tmp_buf)->mode = 1;
                if (OK != safe_put(tmp, tmp_buf, 1, ino)) {
                    return IOERR;
                }
                M_INODE_PTR(inode_buf)->next_inode = tmp;
                if (OK != safe_put(target, inode_buf, 0, ino)) {
                    return IOERR;
                }
            }
            target = M_INODE_PTR(inode_buf)->next_inode;
            safe_get(target, inode_buf, ino);
            index = 0;
        }
        if (!M_INODE_PTR(inode_buf)->direct_blocks[index]) {
            //create next direct block
            tmp = 0;
            memset(tmp_buf, 0, BSIZE);
            safe_put(tmp, tmp_buf, 1, ino);
            M_INODE_PTR(inode_buf)->direct_blocks[index] = tmp;
            safe_put(target, inode_buf, 0, ino);
        }
        if (filesize < 0) {
            memset(tmp_buf, 0, BSIZE);
            safe_put(M_INODE_PTR(inode_buf)->direct_blocks[index], tmp_buf, 0, ino);
        }
        if (off >= BSIZE) {
            off -= BSIZE;
            filesize -= BSIZE;
            ++index;
        } else {
            //off < BSIZE
            break;
        }
    }
    //writing
    while (1) {
        //touch the data block
        if (index >= INODE_DIRECT_BLOCK_NUM) {
            if (!M_INODE_PTR(inode_buf)->next_inode) {
                //create next inode
                tmp = 0;
                memset(tmp_buf, 0, BSIZE);
                M_INODE_PTR(tmp_buf)->mode = 1;
                safe_put(tmp, tmp_buf, 1, ino);
                M_INODE_PTR(inode_buf)->next_inode = tmp;
                safe_put(target, inode_buf, 0, ino);
            }
            target = M_INODE_PTR(inode_buf)->next_inode;
            safe_get(target, inode_buf, ino);
            index = 0;
        }
        if (!M_INODE_PTR(inode_buf)->direct_blocks[index]) {
            //create next direct block
            tmp = 0;
            memset(tmp_buf, 0, BSIZE);
            safe_put(tmp, tmp_buf, 1, ino);
            M_INODE_PTR(inode_buf)->direct_blocks[index] = tmp;
            safe_put(target, inode_buf, 0, ino);
        }
        if (filesize < 0) {
            memset(tmp_buf, 0, BSIZE);
            safe_put(M_INODE_PTR(inode_buf)->direct_blocks[index], tmp_buf, 0, ino);
        }
        //
        if (size <= BSIZE - off) {
            safe_get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf, ino);
            memcpy(data_buf + off, buf, size);
            safe_put(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf, 0, ino);
            buf += size;
            size = 0;
            break;
        } else {
            safe_get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf, ino);
            memcpy(data_buf + off, buf, BSIZE - off);
            safe_put(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf, 0, ino);
            buf += BSIZE - off;
            size -= BSIZE - off;
            filesize -= BSIZE - off;
            off = 0;
            ++index;
        }
    }

    safe_get(ino, inode_buf, ino);
    M_INODE_PTR(inode_buf)->size = new_filesize;
    safe_put(ino, inode_buf, 0, ino);
    size = saved_size;

    return OK;
  return r;
}

int
yfs_client::inner_create(inum parent, const std::string name,inum &ino)
{
  
    ttprintf("inner_create===============> parent: %lld name: %s ino: %lld\n", parent, name.c_str(), ino);
    int ret = OK;
    char buf[BSIZE];
    char tmp_buf[BSIZE];
    inum tmp = 0;
    inum target = parent;

    safe_get(parent, buf, parent);
    M_INODE_PTR(buf)->ctime = M_INODE_PTR(buf)->mtime = time(0);
    if (OK != safe_put(parent, buf, 0, parent)) {
        ret = IOERR;
        goto over;
    }

    //search new blank entry
    while (M_INODE_PTR(buf)->size >= INODE_DIR_ENTRY_NUM) {
        //check duplicated
        for (inum i=0; i<M_INODE_PTR(buf)->size; ++i) {
            if (!strcmp(M_INODE_PTR(buf)->dir_entries[i].name, name.c_str())) {
                ino = (M_INODE_PTR(buf)->dir_entries[i].inum);
                goto over;
            } else {
                ttprintf("inner_create:=> found key %lld value %s\n",
                        M_INODE_PTR(buf)->dir_entries[i].inum,
                        M_INODE_PTR(buf)->dir_entries[i].name);

            }
        }
        //proess
        if (M_INODE_PTR(buf)->next_inode) {
            target = M_INODE_PTR(buf)->next_inode;
            safe_get(target, buf, parent);
        } else {
            memset(tmp_buf, 0, BSIZE);
            if (OK != safe_put(tmp, tmp_buf, 1, parent)) {
                ret = IOERR;
                goto over;
            }
            M_INODE_PTR(buf)->next_inode = tmp;
            if (OK != safe_put(target, buf, 0, parent)) {
                ret = IOERR;
                goto over;
            }
        }
    }
    //check duplicated
    for (inum i=0; i<M_INODE_PTR(buf)->size; ++i) {
        if (!strcmp(M_INODE_PTR(buf)->dir_entries[i].name, name.c_str())) {
            ino = (M_INODE_PTR(buf)->dir_entries[i].inum);
            goto over;
        } else {
            ttprintf("inner_create:=> found key %lld value %s\n",
                    M_INODE_PTR(buf)->dir_entries[i].inum,
                    M_INODE_PTR(buf)->dir_entries[i].name);
        }
    }
    //new file block
    memset(tmp_buf, 0, BSIZE);
    M_INODE_PTR(tmp_buf)->mode = 1;
    M_INODE_PTR(tmp_buf)->atime = 0;
    M_INODE_PTR(tmp_buf)->mtime = time(0);
    M_INODE_PTR(tmp_buf)->ctime = time(0);
    if (OK != safe_put(tmp, tmp_buf, 1, parent)) {
        ret = IOERR;
        goto over;
    }
    //fill in entry
    strcpy(M_INODE_PTR(buf)->dir_entries[M_INODE_PTR(buf)->size].name, name.c_str());
    M_INODE_PTR(buf)->dir_entries[M_INODE_PTR(buf)->size].inum = tmp;
    M_INODE_PTR(buf)->size++;
    if (OK != safe_put(target, buf, 0, parent)) {
        ret = IOERR;
        goto over;
    }
    ino = tmp;
    ret = OK;

over:
    return ret;
}

int
yfs_client::safe_get(inum ino, char *buf, inum ignore)
{
    if (ino == ignore) return get(ino, buf);
    lc->acquire(ino);
    int ret = get(ino, buf);
    lc->release(ino);
    return ret;
}

int
yfs_client::safe_put(inum &ino, char *buf, bool add, inum ignore)
{
    if (add || ino == ignore) return put(ino, buf, add);
    lc->acquire(ino);
    int ret = put(ino, buf, add);
    lc->release(ino);
    return ret;
}
//finding end

