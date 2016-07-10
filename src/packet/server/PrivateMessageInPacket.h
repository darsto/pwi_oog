//
// Created by dar on 7/3/16.
//

#ifndef PWI_OOG_PRIVATEMESSAGEPACKET_H
#define PWI_OOG_PRIVATEMESSAGEPACKET_H

#include <packet/Packet.h>
#include <packet/ServerPacketFactory.h>

SERVER_PACKET(96, PrivateMessageInPacket)

public:
    virtual void prepareData() override {
        stream.read(messageType);
        stream.skipBytes(1);
        stream.read(sender);
        stream.read(senderUID);
        stream.read(recipient);
        stream.read(recipientUID);
        stream.read(msg);
        stream.skipBytes(5);
    }

    byte messageType;
    // [1 byte]
    NString sender;
    unsigned int senderUID;
    NString recipient;
    unsigned int recipientUID;
    NString msg;
    // [5 bytes]
};

#endif //PWI_OOG_PRIVATEMESSAGEPACKET_H
