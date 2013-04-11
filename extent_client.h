// extent client interface.

#ifndef extent_client_h
#define extent_client_h

#include <string>
#include "extent_protocol.h"
#define BSIZE 512
class extent_client {
    //filling begin
    private:
        int fd;
        int mutex;
    //filling end
    public:
        extent_client();

        extent_protocol::status get(extent_protocol::extentid_t eid, 
                char* buf);
        extent_protocol::status put(extent_protocol::extentid_t eid, char* buf);
        extent_protocol::status remove(extent_protocol::extentid_t eid);
        //filling begin
        ~extent_client();
        //filling end
};

#endif 

