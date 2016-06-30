//
// Created by dar on 6/30/16.
//

#ifndef PWI_OOG_SELECTROLECONFIRMATIONPACKET_H
#define PWI_OOG_SELECTROLECONFIRMATIONPACKET_H

#include <packet/Packet.h>
#include <packet/ServerPacketFactory.h>

SERVER_PACKET(71, SelectRoleConfirmationPacket)

public:
    virtual void prepareData() override {
        stream.skipBytes(5);
    }
};

#endif //PWI_OOG_SELECTROLECONFIRMATIONPACKET_H
