//
// Created by dar on 6/25/16.
//

#ifndef PWI_OOG_ONLINEANNOUNCE_H
#define PWI_OOG_ONLINEANNOUNCE_H

#include <packet/Packet.h>
#include <packet/ServerPacketFactory.h>

SERVER_PACKET(4, OnlineAnnouncePacket)
public:
    virtual void prepareData() override {
        stream.read(accoundId);
        stream.read(unkId);
        unknown1 = stream.readArray<byte>(21);
    }

    unsigned int accoundId;
    unsigned int unkId;
    std::vector<byte> unknown1;
};

#endif //PWI_OOG_ONLINEANNOUNCE_H
