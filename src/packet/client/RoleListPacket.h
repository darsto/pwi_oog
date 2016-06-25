//
// Created by dar on 6/25/16.
//

#ifndef PWI_OOG_ROLELIST_H
#define PWI_OOG_ROLELIST_H

#include <packet/Packet.h>

class RoleListPacket : public Packet {
public:
    static const int ID = 2;

    RoleListPacket(DataStream &stream,
                   ConnectionData &connectionData,
                   int slot)
        : Packet(stream, connectionData), slot(slot) {}

    virtual void prepareData() override {
        stream.write(connectionData.accountId);
        stream.write((int) 0);
        stream.write(slot);
    }

    int slot;
};

#endif //PWI_OOG_ROLELIST_H
