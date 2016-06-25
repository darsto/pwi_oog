//
// Created by dar on 6/16/16.
//

#ifndef PWI_OOG_SMKEYPACKET_H
#define PWI_OOG_SMKEYPACKET_H

#include <packet/Packet.h>
#include <md5.h>
#include <hmac.h>
#include <HexConverter.h>
#include <packet/ServerPacketFactory.h>

SERVER_PACKET(2, SMKeyPacket)

public:
    virtual void prepareData() override {
        SMKey = stream.readArray<byte>();
        force = stream.read<byte>();
    }

    std::vector<byte> SMKey;
    bool force;
};

#endif //PWI_OOG_SMKEYPACKET_H
