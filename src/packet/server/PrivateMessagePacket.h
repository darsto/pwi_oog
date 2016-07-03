//
// Created by dar on 7/3/16.
//

#ifndef PWI_OOG_PRIVATEMESSAGEPACKET_H
#define PWI_OOG_PRIVATEMESSAGEPACKET_H

#include <packet/Packet.h>
#include <packet/ServerPacketFactory.h>

SERVER_PACKET(96, PrivateMessagePacket)

public:
    virtual void prepareData() override {
        stream.read(messageType);
        stream.skipBytes(1);
        stream.read(sender);
        stream.read(senderUID);
        stream.read(name);
        stream.read(UID);
        stream.read(msg);
    }

    byte messageType;
    // [1 byte]
    std::string sender;
    unsigned int senderUID;
    std::string name;
    unsigned int UID;
    std::string msg;
};

#endif //PWI_OOG_PRIVATEMESSAGEPACKET_H
