//
// Created by dar on 6/26/16.
//

#ifndef PWI_OOG_KEEPALIVEPACKET_H
#define PWI_OOG_KEEPALIVEPACKET_H

#include <packet/Packet.h>

class KeepAlivePacket : public Packet {
public:
    static const int ID = 90;

    KeepAlivePacket(DataStream &stream, ConnectionData &connectionData)
        : Packet(stream, connectionData) {}

    virtual void prepareData() override {
        stream.write(unknown1);
    }

    byte unknown1 = ID;
};

#endif //PWI_OOG_KEEPALIVEPACKET_H
