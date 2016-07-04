//
// Created by dar on 7/4/16.
//

#ifndef PWI_OOG_PRIVATEMESSAGEOUTPACKET_H
#define PWI_OOG_PRIVATEMESSAGEOUTPACKET_H

#include <packet/Packet.h>

class PrivateMessageOutPacket : public Packet {
public:
    static const int ID = 96;

    PrivateMessageOutPacket(DataStream &stream,
                            ConnectionData &connectionData,
                            const std::string &recipient,
                            const std::string &message)
        : Packet(stream, connectionData), recipient(recipient), message(message) {}

    virtual void prepareData() override {
        stream.write(type);
        stream.write(emotion);
        stream.write(connectionData.selectedRole.name);
        stream.write(connectionData.selectedRole.UID);
        stream.write(recipient);
        stream.write(recipientUID);
        stream.write(message);
        stream.write(unknown1);
        stream.write(unknown2);
    }

    byte type;
    byte emotion;
    //std::string sender;
    //unsigned int senderUID;
    const std::string &recipient;
    unsigned int recipientUID;
    const std::string &message;
    byte unknown1;
    byte unknown2;
};

#endif //PWI_OOG_PRIVATEMESSAGEOUTPACKET_H
