//
// Created by dar on 6/27/16.
//

#ifndef PWI_OOG_PINGPACKET_H
#define PWI_OOG_PINGPACKET_H

#include <packet/Packet.h>
#include <md5.h>
#include <hmac.h>
#include <HexConverter.h>
#include <packet/ServerPacketFactory.h>

SERVER_PACKET(90, PingPacket)

public:
    virtual void prepareData() override {
        stream.read(unknown1);
    }

    byte unknown1;
};

#endif //PWI_OOG_PINGPACKET_H
