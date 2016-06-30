//
// Created by dar on 6/30/16.
//

#ifndef PWI_OOG_SELECTROLEPACKET_H
#define PWI_OOG_SELECTROLEPACKET_H

#include <packet/Packet.h>
#include <pwi/RoleInfo.h>

class SelectRolePacket : public Packet {
public:
    static const int ID = 70;

    SelectRolePacket(DataStream &stream,
                          ConnectionData &connectionData,
                          RoleInfo &roleInfo)
        : Packet(stream, connectionData), roleInfo(roleInfo) {}

    virtual void prepareData() override {
        stream.write(roleInfo.UID);
        stream.write((byte) 0);
    }

    RoleInfo &roleInfo;
};

#endif //PWI_OOG_SELECTROLEPACKET_H
