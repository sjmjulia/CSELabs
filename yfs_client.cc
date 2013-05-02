// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include "lock_client.h"
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
  lc = new lock_client(lock_dst);
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
    ec->put(0, buf);
    /////////////////////BLOCK-BITMAP from 10 ~ 200
    memset(buf, 0, BSIZE);
    ec->put(BLOCK_BITMAP_END_BLOCK, buf);
    *(buf) = 1;
    *(buf + (1 >> 3)) |= 1 << (1 & 0x7);
    for (int i=BLOCK_BITMAP_BEGIN_BLOCK; i<=BLOCK_BITMAP_END_BLOCK; ++i) {
        *(buf + (i >> 3)) |= 1 << (i & 0x7);
    }
    ec->put(BLOCK_BITMAP_BEGIN_BLOCK, buf);

    // root block
    memset(buf, 0, BSIZE);
    ((inode*)buf)->mode = 0;
    ((inode*)buf)->size = 0;
    ((inode*)buf)->atime = 0;
    ((inode*)buf)->mtime = 0;
    ((inode*)buf)->ctime = 0;
    ec->put(1, buf);
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
    if (OK != get(inum, buf)) {
        ret = false;
        goto over;
    }
    ret = (1==M_INODE_PTR(buf)->mode);
    //filling end
over:
    lc->release(inum);
    return ret;
}

bool
yfs_client::isdir(inum inum)
{
    //filling begin
    char buf[BSIZE];
    bool ret = false;
    lc->acquire(inum);
    if (OK != get(inum, buf)) {
        ret = false;
        goto over;
    }
    ret = (0==M_INODE_PTR(buf)->mode);
over:
    lc->release(inum);
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
    lc->acquire(inum);
    int ret = get(inum, buf);
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
    int ret = get(inum, buf);
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
    return ret;
}


//filling begin
int yfs_client::get(inum ino, char *buf) {
    if (ino < BLOCK_BITMAP_BEGIN_BLOCK || ino > BLOCK_BITMAP_END_BLOCK) {
        //char super_block_buf[BSIZE];
        //get(0, super_block_buf);
        //if (!((*(super_block_buf + SUPER_BLOCK_BLOCK_BITMAP_OFFSET + (ino >> 3)) >> (ino & 0x7)) & 1)) {
        //    return NOENT;
        //}
        inum bitmap_block = BLOCK_BITMAP_BEGIN_BLOCK + ino / BITMAP_PER_BLOCK;
        inum bitmap = ino % BITMAP_PER_BLOCK;
        char bitmap_block_buf[BSIZE];
        get(bitmap_block, bitmap_block_buf);
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
            get(bitmap_block, bitmap_block_buf);
            if (!((*(bitmap_block_buf + (bitmap >> 3)) >> (bitmap & 0x7)) & 1)) {
                if (OK != ec->put(i, buf)) {
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
        inum bitmap_block = BLOCK_BITMAP_BEGIN_BLOCK + ino / BITMAP_PER_BLOCK;
        inum bitmap = ino % BITMAP_PER_BLOCK;
        char bitmap_block_buf[BSIZE];
        get(bitmap_block, bitmap_block_buf);
        if (!((*(bitmap_block_buf + (bitmap >> 3)) >> (bitmap & 0x7)) & 1)) {
            return NOENT;
        }
        ec->put(ino, buf);
        return OK;
    }
}

int yfs_client::remove(inum ino) {
    inum bitmap_block = BLOCK_BITMAP_BEGIN_BLOCK + ino / BITMAP_PER_BLOCK;
    lc->acquire(bitmap_block);
    inum bitmap = ino % BITMAP_PER_BLOCK;
    char bitmap_block_buf[BSIZE];
    get(bitmap_block, bitmap_block_buf);
    *(bitmap_block_buf + (bitmap >> 3)) &= ~(1 << (bitmap & 0x7));
    int ret = IOERR;
    if (OK != put(bitmap_block, bitmap_block_buf, 0)) {
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
  
    int ret = OK;
    char buf[BSIZE];
    char tmp_buf[BSIZE];
    inum tmp = 0;
    inum target = parent;

    lc->acquire(parent);                                            //+
    get(parent, buf);
    M_INODE_PTR(buf)->ctime = M_INODE_PTR(buf)->mtime = time(0);
    if (OK != put(parent, buf, 0)) {
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
            }
        }
        //proess
        if (M_INODE_PTR(buf)->next_inode) {
            target = M_INODE_PTR(buf)->next_inode;
            get(target, buf);
        } else {
            memset(tmp_buf, 0, BSIZE);
            if (OK != put(tmp, tmp_buf, 1)) {
                ret = IOERR;
                goto over;
            }
            M_INODE_PTR(buf)->next_inode = tmp;
            if (OK != put(target, buf, 0)) {
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
        }
    }
    //new file block
    memset(tmp_buf, 0, BSIZE);
    M_INODE_PTR(tmp_buf)->mode = 1;
    M_INODE_PTR(tmp_buf)->atime = 0;
    M_INODE_PTR(tmp_buf)->mtime = time(0);
    M_INODE_PTR(tmp_buf)->ctime = time(0);
    if (OK != put(tmp, tmp_buf, 1)) {
        ret = IOERR;
        goto over;
    }
    //fill in entry
    strcpy(M_INODE_PTR(buf)->dir_entries[M_INODE_PTR(buf)->size].name, name.c_str());
    M_INODE_PTR(buf)->dir_entries[M_INODE_PTR(buf)->size].inum = tmp;
    M_INODE_PTR(buf)->size++;
    if (OK != put(target, buf, 0)) {
        ret = IOERR;
        goto over;
    }
    ino = tmp;
    ret = OK;

over:
    lc->release(parent);                                    //-
    return ret;
}

yfs_client::inum
yfs_client::lookup(inum parent, const std::string name)
{
    inum ret = 0;
    char buf[BSIZE];

    lc->acquire(parent);                                    //+

    get(parent, buf);
    M_INODE_PTR(buf)->atime = time(0);
    if (OK != put(parent, buf, 0)) {
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
        get(M_INODE_PTR(buf)->next_inode, buf);
    }

over:
    lc->release(parent);
    return ret;
}

int yfs_client::readdir(inum ino,std::map<inum,std::string> &items)
{
    int ret = IOERR;
    char buf[BSIZE];

    lc->acquire(ino);                                       //+
    items.clear();
    get(ino, buf);
    M_INODE_PTR(buf)->atime = time(0);
    if (OK != put(ino, buf, 0)) {
        ret = IOERR;
        goto over;
    }
    //search new blank entry
    while (1){
        for (inum i=0; i<M_INODE_PTR(buf)->size; ++i) {
            items[M_INODE_PTR(buf)->dir_entries[i].inum]
                = std::string(M_INODE_PTR(buf)->dir_entries[i].name);
        }
        if (!M_INODE_PTR(buf)->next_inode) break;
        get(M_INODE_PTR(buf)->next_inode, buf);
    }
    ret = OK;
over:
    lc->release(ino);                                       //-
    return ret;
}

int yfs_client::setattr(inum ino,int to_set,const fileinfo &info)
{
    int ret = OK;
    char buf[BSIZE];
    lc->acquire(ino);                                       //+
    get(ino, buf);
    M_INODE_PTR(buf)->atime = info.atime;
    M_INODE_PTR(buf)->mtime = info.mtime;
    M_INODE_PTR(buf)->ctime = info.ctime;
    if (to_set) {
        inum tmp = ino;
        size_t write_size = 0;
        inner_write(tmp, (const char *)buf, write_size, (off_t)info.size);
        M_INODE_PTR(buf)->size = info.size;
    }
    if (OK != put(ino, buf, 0)) {
        ret = IOERR;
        goto over;
    }
over:
    lc->release(ino);                                       //-
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
    get(parent, buf);
    M_INODE_PTR(buf)->ctime = time(0);
    M_INODE_PTR(buf)->mtime = time(0);
    put(parent, buf, 0);

    inum target = parent;
    //search new blank entry
    while (M_INODE_PTR(buf)->size >= INODE_DIR_ENTRY_NUM) {
        /////////////////check duplicate ?
        if (M_INODE_PTR(buf)->next_inode) {
            target = M_INODE_PTR(buf)->next_inode;
            get(target, buf);
        } else {
            memset(tmp_buf, 0, BSIZE);
            put(tmp, tmp_buf, 1);
            M_INODE_PTR(buf)->next_inode = tmp;
            put(target, buf, 0);
        }
    }
    //new file block
    memset(tmp_buf, 0, BSIZE);
    M_INODE_PTR(tmp_buf)->atime = 0;
    M_INODE_PTR(tmp_buf)->mtime = time(0);
    M_INODE_PTR(tmp_buf)->ctime = time(0);
    put(tmp, tmp_buf, 1);
    //fill in entry
    strcpy(M_INODE_PTR(buf)->dir_entries[M_INODE_PTR(buf)->size].name, name.c_str());
    M_INODE_PTR(buf)->dir_entries[M_INODE_PTR(buf)->size].inum = tmp;
    M_INODE_PTR(buf)->size++;
    put(target, buf, 0);
    ino = tmp;

    lc->release(parent);
    return OK;
}

int
yfs_client::unlink(inum parent, const std::string name)
{
    int ret = OK;

    char buf[BSIZE];
    inum unlink_inum = 0;
    inum buf_inum = parent;
    
    lc->acquire(parent);

    get(parent, buf);
    M_INODE_PTR(buf)->mtime = time(0);
    M_INODE_PTR(buf)->ctime = time(0);
    put(parent, buf, 0);
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
        get(buf_inum, buf);
    }
    if (!found) {
        printf("unlink: not found!\n");
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
    put(buf_inum, new_buf, 0);

    recursive_unlink(unlink_inum);
over:
    lc->release(parent);
    printf("sfsdddddddddddddddddddddddddddddddddddddddddddddddddddddddd %d\n", ret);
    return ret;
}

//filling begin
int yfs_client::recursive_unlink(inum ino) {
    if (!ino) return 0;
    char buf[BSIZE];
    get(ino, buf);
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
    if (OK != get(ino, buf)) {
        return false;
    }
    return 2==M_INODE_PTR(buf)->mode;
}

int
yfs_client::symlink(inum parent, const std::string name, 
        const std::string link, inum &ino)
{
    std::cout << "sdf sd " << std::endl;
    inum tmp;
    int ret = create(parent, name, tmp);
    std::cout << "createdsd " << std::endl;
    size_t len = link.size();

    if (OK != ret) goto ret_pnt;

    ret = inner_write(tmp, link.c_str(), len, (off_t)0);
    std::cout << "writedcr" << std::endl;

    if (OK != ret) goto ret_pnt;;

    char buf[BSIZE];
    if (OK != get(tmp, buf)) goto ret_pnt;

    std::cout << "moded" << std::endl;
    M_INODE_PTR(buf)->mode = 2;
    if (OK != put(tmp, buf, 0)) goto ret_pnt;

    ino = tmp;
ret_pnt:
    return ret;
}

int
yfs_client::readlink(inum ino, std::string &link)
{
    
    int ret = 0;
    fileinfo info;

    getfile(ino, info);
    ret = inner_read(ino, info.size, 0, link);

    return ret;
}
int yfs_client::inner_read(inum ino,size_t size,off_t off,std::string &buf)
{
    int r = IOERR;
    char inode_buf[BSIZE];
    char data_buf[BSIZE];
    buf = "";
    if (0 == size) return OK;

    if (NOENT == get(ino, inode_buf)) {
        return NOENT;
    }
    M_INODE_PTR(inode_buf)->atime = time(0);
    if (OK != put(ino, inode_buf, 0)) {
        return IOERR;
    }
    inum filesize = M_INODE_PTR(inode_buf)->size;
    inum index = 0;
    //offsetting
    while (1) {
        //touch the data block
        if (index >= INODE_DIRECT_BLOCK_NUM) {
            if (M_INODE_PTR(inode_buf)->next_inode) {
                get(M_INODE_PTR(inode_buf)->next_inode, inode_buf);
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
                get(M_INODE_PTR(inode_buf)->next_inode, inode_buf);
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
                get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf);
                //may be a \0 in data
                buf.append(data_buf + off, size);
                filesize -= size;
                size = 0;
                break;
            } else {
                get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf);
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
                get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf);
                //may be a \0 in data
                buf.append(data_buf + off, filesize);
                size -= filesize;
                filesize = 0;
                break;
            } else {
                get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf);
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

    if (NOENT == get(ino, inode_buf)) {
        return NOENT;
    }
    M_INODE_PTR(inode_buf)->atime = time(0);
    M_INODE_PTR(inode_buf)->mtime = time(0);
    if (OK != put(ino, inode_buf, 0)) {
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
                if (OK != put(tmp, tmp_buf, 1)) {
                    return IOERR;
                }
                M_INODE_PTR(inode_buf)->next_inode = tmp;
                if (OK != put(target, inode_buf, 0)) {
                    return IOERR;
                }
            }
            target = M_INODE_PTR(inode_buf)->next_inode;
            get(target, inode_buf);
            index = 0;
        }
        if (!M_INODE_PTR(inode_buf)->direct_blocks[index]) {
            //create next direct block
            tmp = 0;
            memset(tmp_buf, 0, BSIZE);
            put(tmp, tmp_buf, 1);
            M_INODE_PTR(inode_buf)->direct_blocks[index] = tmp;
            put(target, inode_buf, 0);
        }
        if (filesize < 0) {
            memset(tmp_buf, 0, BSIZE);
            put(M_INODE_PTR(inode_buf)->direct_blocks[index], tmp_buf, 0);
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
                put(tmp, tmp_buf, 1);
                M_INODE_PTR(inode_buf)->next_inode = tmp;
                put(target, inode_buf, 0);
            }
            target = M_INODE_PTR(inode_buf)->next_inode;
            get(target, inode_buf);
            index = 0;
        }
        if (!M_INODE_PTR(inode_buf)->direct_blocks[index]) {
            //create next direct block
            tmp = 0;
            memset(tmp_buf, 0, BSIZE);
            put(tmp, tmp_buf, 1);
            M_INODE_PTR(inode_buf)->direct_blocks[index] = tmp;
            put(target, inode_buf, 0);
        }
        if (filesize < 0) {
            memset(tmp_buf, 0, BSIZE);
            put(M_INODE_PTR(inode_buf)->direct_blocks[index], tmp_buf, 0);
        }
        //
        if (size <= BSIZE - off) {
            get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf);
            memcpy(data_buf + off, buf, size);
            put(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf, 0);
            buf += size;
            size = 0;
            break;
        } else {
            get(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf);
            memcpy(data_buf + off, buf, BSIZE - off);
            put(M_INODE_PTR(inode_buf)->direct_blocks[index], data_buf, 0);
            buf += BSIZE - off;
            size -= BSIZE - off;
            filesize -= BSIZE - off;
            off = 0;
            ++index;
        }
    }

    get(ino, inode_buf);
    M_INODE_PTR(inode_buf)->size = new_filesize;
    put(ino, inode_buf, 0);
    size = saved_size;

    return OK;
  return r;
}

//finding end
