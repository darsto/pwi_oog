//
// Created by dar on 6/16/16.
//

#ifndef PWI_OOG_LOGINPACKET_H
#define PWI_OOG_LOGINPACKET_H

#include <packet/Packet.h>
#include <md5.h>
#include <HexConverter.h>
#include <hmac.h>

class LoginPacket : public Packet {
public:
    static const int ID = 3;

    LoginPacket(DataStream &stream,
                ConnectionData &connectionData,
                const std::string &login,
                const std::string &password)
        : Packet(stream, connectionData),
          login(login),
          password(password),
          unknown2(4) {
        MD5 md5;
        auto loginDataHashed = hexToDec(md5(login + password));
        std::string authHash_hex = hmac<MD5>(connectionData.serverKey.data(), connectionData.serverKey.size(), loginDataHashed.data(), loginDataHashed.size());
        connectionData.authHash = hexToDec(authHash_hex);
    }

    void prepareData() override {
        stream.write(login);
        stream.write(connectionData.authHash);
        stream.write(unknown1);
        stream.write(unknown2);
    }

    std::string login;
    std::string password;
    byte unknown1 = 0;
    std::vector<byte> unknown2;

};

#endif //PWI_OOG_LOGINPACKET_H
