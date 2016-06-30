//
// Created by dar on 6/30/16.
//

#ifndef PWI_OOG_ENTERWORLDPACKET_H
#define PWI_OOG_ENTERWORLDPACKET_H

#include <packet/Packet.h>
#include <pwi/RoleInfo.h>

class EnterWorldPacket : public Packet {
public:
    static const int ID = 72;

    EnterWorldPacket(DataStream &stream,
                     ConnectionData &connectionData,
                     RoleInfo &roleInfo)
        : Packet(stream, connectionData), roleInfo(roleInfo) {}

    virtual void prepareData() override {
        stream.write(roleInfo.UID);
        stream.skipBytes(20);
    }

    RoleInfo &roleInfo;
};

#endif //PWI_OOG_ENTERWORLDPACKET_H
