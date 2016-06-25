//
// Created by dar on 6/16/16.
//

#ifndef PWI_OOG_CMKEYPACKET_H
#define PWI_OOG_CMKEYPACKET_H

#include <packet/Packet.h>

class CMKeyPacket : public Packet {
public:
    static const int ID = 2;

    CMKeyPacket(DataStream &stream,
                ConnectionData &connectionData,
                bool force)
        : Packet(stream, connectionData), force(force) {}

    virtual void prepareData() override {
        stream.writeArray(connectionData.CMKey);
        stream.write((byte) force);
    }

    bool force;
};

#endif //PWI_OOG_CMKEYPACKET_H
