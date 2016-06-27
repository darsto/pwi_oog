//
// Created by dar on 6/27/16.
//

#ifndef PWI_OOG_LASTLOGININFOPACKET_H
#define PWI_OOG_LASTLOGININFOPACKET_H

#include <packet/Packet.h>
#include <packet/ServerPacketFactory.h>

SERVER_PACKET(143, LastLoginInfoPacket)
public:
    virtual void prepareData() override {
        stream.read(accoundId);
        stream.read(unkId);
        stream.read(lastLoginTime);
        lastLoginIP = stream.readArray(4);
        currentIP = stream.readArray(4);
    }

    unsigned int accoundId;
    unsigned int unkId;
    unsigned int lastLoginTime;
    std::vector<byte> lastLoginIP;
    std::vector<byte> currentIP;
};

#endif //PWI_OOG_LASTLOGININFOPACKET_H
