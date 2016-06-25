//
// Created by dar on 6/16/16.
//

#ifndef PWI_OOG_NETWORKMANAGER_H
#define PWI_OOG_NETWORKMANAGER_H

#include <packet/Packet.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <packet/PacketManager.h>
#include <packet/ServerPacketFactory.h>
#include <packet/client/CMKeyPacket.h>
#include <packet/client/LoginPacket.h>
#include <packet/server/ServerInfoPacket.h>
#include <packet/server/SMKeyPacket.h>
#include <packet/server/OnlineAnnouncePacket.h>

class ConnectionManager {
public:
    static const int MAX_BYTES = 1024;
    using data_type = std::shared_ptr<std::array<byte, MAX_BYTES>>;

    ConnectionManager(const std::string &hostAddress, uint16_t port)
        : hostAddress(hostAddress),
          port(port),
          receiveBuffer(std::make_shared<std::array<byte, MAX_BYTES>>()),
          sendBuffer(std::make_shared<std::array<byte, MAX_BYTES>>()) {
        host = gethostbyname(hostAddress.c_str());
        sock = socket(AF_INET, SOCK_STREAM, 0);

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr = *((struct in_addr *) host->h_addr);
        bzero(&(serverAddress.sin_zero), 8);
    }

    bool initConnection() {
        connected = !connect(sock, (struct sockaddr *) &serverAddress, sizeof(struct sockaddr));
        return connected;
    }

    void listen() {
        ssize_t bytes_recieved = recv(sock, receiveBuffer.get(), MAX_BYTES, 0);
        int b2 = bytes_recieved;
        connectionData.cipher.decrypt(receiveBuffer.get()->data(), bytes_recieved);
        if (bytes_recieved > 0) {
            printf("Received data [%d] = ", bytes_recieved - 2);
            for (int i = 0; i < bytes_recieved; ++i) {
                printf("%d ", (int) ((unsigned char) (*receiveBuffer.get())[i]));
            }
            printf("\n");
            DataStream dataStream(receiveBuffer, bytes_recieved);
            const int packetId = dataStream.read<byte>();
            const int packetSize = dataStream.read<byte>();
            try {
                auto p1 = PacketManager::getInstance().getFactory(PacketType::SERVER_DEFAULT, packetId)->createPacket(dataStream, this->connectionData);
                if (p1->getStream().getLength() - 2 != packetSize) printf("Received packet [%d] contains unread data (%d)\n", packetId, packetSize - p1->getStream().getLength() + 2);
            } catch (const std::out_of_range &e) {
                printf("%s\n", e.what());
                exit(0);
            }
        }
    }

    template<class TPacket, typename... Args>
    void sendPacket(Args &&... args) {
        DataStream stream(sendBuffer);
        stream.write((byte) TPacket::ID);
        stream.write((byte) 0);
        TPacket packet(stream, std::forward<Args>(args)...);
        packet.prepareData();
        packet.getStream().getBuffer()->at(1) = (byte) (packet.getStream().getLength() - 2);
        connectionData.cipher.encrypt(packet.getStream().getBuffer().get()->data(), packet.getStream().getLength());
        packet.getStream().seal();
        sendPacket(packet);
    }

    void sendPacket(const Packet &packet) {
        if (!packet.getStream().isSealed()) throw std::runtime_error("Trying to send an unsealed packet");
        printf("Tring to send data [%d] = ", packet.getStream().getLength() - 2);
        for (int i = 0; i < packet.getStream().getLength(); ++i) {
            printf("%d ", (int) ((unsigned char) (*packet.getStream().getBuffer().get())[i]));
        }
        printf("\n");
        ssize_t bytes_sent = send(sock, (*packet.getStream().getBuffer().get()).data(), packet.getStream().getLength(), 0);
        if (bytes_sent != packet.getStream().getLength()) printf("Sent packet with an unexpected size\n");
    }

    ConnectionData connectionData;

private:
    const std::string hostAddress;
    const uint16_t port;
    hostent *host; //memory is handled internally by the socket library
    sockaddr_in serverAddress;
    int sock;
    bool connected = false;
    data_type receiveBuffer;
    data_type sendBuffer;
};

#endif //PWI_OOG_NETWORKMANAGER_H
