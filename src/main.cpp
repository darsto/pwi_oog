#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <vector>
#include <packet/Packet.h>
#include <packet/client/RoleListRequestPacket.h>
#include <chrono>
#include <thread>
#include <packet/client/KeepAlivePacket.h>
#include <packet/client/SelectRolePacket.h>
#include <packet/client/EnterWorldPacket.h>
#include <packet/client/PrivateMessageOutPacket.h>
#include <iostream>
#include "ConnectionManager.h"
#include "packet/ServerPacketFactory.h"

std::string login = "YOUR_LOGIN";
std::string password = "YOUR_PASSWORD";

int main() {
    ConnectionManager connectionManager("pwieu3.en.perfectworld.eu", 29000); //connect to EU - Dawn Glory

    bool connected = connectionManager.initConnection();

    if (connected) {
        bool running = true;

        PacketManager &packetManager = PacketManager::getInstance();
        ConnectionData &connectionData = connectionManager.connectionData;

        packetManager.registerHandler<ServerInfoPacket>([&connectionManager, &packetManager, &connectionData](Packet &p) {
            ServerInfoPacket &packet = static_cast<ServerInfoPacket &>(p);

            if (packet.hashToSha256) printf("Server key is encrypted\n");

            connectionData.serverKey = std::move(packet.key);
            connectionData.serverVersion = std::move(packet.serverVersion);
            connectionData.crc = std::move(packet.crc);

            connectionManager.sendPacket<LoginPacket>(connectionData, login, password);
        });

        packetManager.registerHandler<SMKeyPacket>([&connectionManager, &packetManager, &connectionData](Packet &p) {
            SMKeyPacket &packet = static_cast<SMKeyPacket &>(p);

            connectionData.SMKey = std::move(packet.SMKey);

            srand((unsigned int) time(NULL));
            std::vector<byte> CMKey(16);
            for (int i = 0; i < 16; ++i) {
                CMKey[i] = (byte) (rand() % 256);
            }
            connectionData.CMKey = std::move(CMKey);

            size_t decSize = connectionData.authHash.size() + connectionData.CMKey.size();
            std::unique_ptr<byte[]> decKey = std::make_unique<byte[]>(decSize);
            for (int i = 0; i < connectionData.authHash.size(); ++i) {
                decKey.get()[i] = connectionData.authHash[i];
            }
            for (int i = 0; i < connectionData.CMKey.size(); ++i) {
                decKey.get()[connectionData.authHash.size() + i] = connectionData.CMKey[i];
            }

            std::string decKeyHashed_hex = hmac<MD5>(decKey.get(), decSize, login.data(), login.size());

            size_t encSize = connectionData.authHash.size() + connectionData.SMKey.size();
            std::unique_ptr<byte[]> encKey = std::make_unique<byte[]>(encSize);
            for (int i = 0; i < connectionData.authHash.size(); ++i) {
                encKey.get()[i] = connectionData.authHash[i];
            }
            for (int i = 0; i < connectionData.SMKey.size(); ++i) {
                encKey.get()[connectionData.authHash.size() + i] = connectionData.SMKey[i];
            }

            std::string encKeyHashed_hex = hmac<MD5>(encKey.get(), encSize, login.data(), login.size());

            auto encKeyHashed = hexToDec(encKeyHashed_hex);
            auto decKeyHashed = hexToDec(decKeyHashed_hex);

            connectionData.cipher.init(encKeyHashed, decKeyHashed);

            connectionManager.sendPacket<CMKeyPacket>(connectionData, packet.force);
        });

        packetManager.registerHandler<OnlineAnnouncePacket>([&connectionManager, &packetManager, &connectionData](Packet &p) {
            OnlineAnnouncePacket &packet = static_cast<OnlineAnnouncePacket &>(p);
            connectionData.accountId = std::move(packet.accountId);

            std::thread *t1 = new std::thread([&connectionManager]() {
                while (true) {
                    connectionManager.sendPacket<KeepAlivePacket>(connectionManager.connectionData);
                    std::this_thread::sleep_for(std::chrono::seconds(15));
                }
            });
            std::this_thread::sleep_for(std::chrono::seconds(1));
            connectionManager.sendPacket<RoleListRequestPacket>(connectionData, -1);
        });

        packetManager.registerHandler<RoleListPacket>([&connectionManager, &packetManager, &connectionData](Packet &p) {
            RoleListPacket &packet = static_cast<RoleListPacket &>(p);
            printf("Character: %s\n", packet.roleInfo.name.get().c_str());

            if (packet.isChar) {
                connectionData.selectedRole = std::move(packet.roleInfo); //todo select
                connectionManager.sendPacket<RoleListRequestPacket>(connectionData, packet.nextSlot);
            } else {
                connectionManager.sendPacket<SelectRolePacket>(connectionData, connectionData.selectedRole);
            }
        });

        packetManager.registerHandler<SelectRoleConfirmationPacket>([&connectionManager, &packetManager, &connectionData](Packet &p) {
            SelectRoleConfirmationPacket &packet = static_cast<SelectRoleConfirmationPacket &>(p);
            connectionManager.sendPacket<EnterWorldPacket>(connectionData, connectionData.selectedRole);
        });

        packetManager.registerHandler<PrivateMessageInPacket>([&connectionManager, &packetManager, &connectionData](Packet &p) {
            PrivateMessageInPacket &packet = static_cast<PrivateMessageInPacket &>(p);
            printf("%s says: %s\n", packet.sender.get().c_str(), packet.msg.get().c_str());
        });

        std::thread inputThr([&running, &connectionManager] () {
           while (running) {
               std::string nick;
               std::cin >> nick;

               std::string msg;
               std::getline(std::cin, msg);

               connectionManager.sendPacket<PrivateMessageOutPacket>(connectionManager.connectionData, nick, msg);
           }
        });

        while (running) {
            connectionManager.listen();
            usleep(10 * 1000);
        }
    }

    return 0;
}
