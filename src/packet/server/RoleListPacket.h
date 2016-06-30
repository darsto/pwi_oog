//
// Created by dar on 6/27/16.
//

#ifndef PWI_OOG_ROLELISTPACKET_H
#define PWI_OOG_ROLELISTPACKET_H

#include <packet/Packet.h>
#include <packet/ServerPacketFactory.h>
#include <pwi/RoleInfo.h>

SERVER_PACKET(83, RoleListPacket)
public:
    virtual void prepareData() override {
        stream.skipBytes(4);
        stream.read(nextSlot);
        stream.read(accountId);
        stream.skipBytes(4);
        stream.read(isChar);
        if (isChar) stream.read(roleInfo);
    }

    //[4 bytes]
    int nextSlot;
    unsigned int accountId;
    //[4 bytes]
    bool isChar;
    RoleInfo roleInfo;
    //more unknown content of unknown length (!)
};

#endif //PWI_OOG_ROLELISTPACKET_H
