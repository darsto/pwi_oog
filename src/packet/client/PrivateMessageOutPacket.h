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

        std::vector<char> message_encoded(message.length() * 2);
        for (int i = 0; i < message.length(); ++i) {
            message_encoded[i * 2] = message[i];
        }

        stream.write(message_encoded);
        stream.skipBytes(5);
    }

    byte type = 0;
    byte emotion = 0;
    //std::string sender;
    //unsigned int senderUID;
    const std::string &recipient;
    unsigned int recipientUID = 0;
    const std::string &message;
    //[5 bytes]
};

#endif //PWI_OOG_PRIVATEMESSAGEOUTPACKET_H
