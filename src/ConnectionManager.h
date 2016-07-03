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
#include <packet/server/LastLoginInfoPacket.h>
#include <packet/server/RoleListPacket.h>
#include <packet/server/PingPacket.h>
#include <packet/server/SelectRoleConfirmationPacket.h>

class ConnectionManager {
public:
    ConnectionManager(const std::string &hostAddress, uint16_t port)
        : hostAddress(hostAddress),
          port(port) {
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
        DataStream dataStream;
        ssize_t bytes_received = 0;
        do {
            DataStream::DataChunk bufferPtr;
            bytes_received = recv(sock, bufferPtr.data(), 1024, 0);

            if (bytes_received <= 0) return;

            printf("Actual bytes received: %d\n", bytes_received);

            auto bytes_unpacked = connectionData.cipher.decrypt(dataStream, bufferPtr, bytes_received);

            printf("After unpacking: %d\n", bytes_unpacked);
        } while (bytes_received == 1024); //TODO || bytes_received < 0 (?)
        dataStream.seal();

        printf("Received data [%d] = ", (int) dataStream.getLength());
        for (int i = 0; i < dataStream.getLength(); ++i) {
            printf("%d ", (int) dataStream.getByteAt(i));
        }
        printf("\n");

        while (dataStream.getPos() < dataStream.getLength()) { //single stream may contain multiple packets
            uni_int packetId = dataStream.read<uni_int>();
            uni_int packetSize = dataStream.read<uni_int>();

            if (packetId == 0) {
                dataStream.setSwapped(false);
                //TODO read container
                break;
            } else {
                printf("Handling packet [%d] [%d]\n", packetId, packetSize);
                size_t previousPosition = dataStream.getPos();
                try {
                    auto p1 = PacketManager::getInstance().getFactory(PacketType::SERVER_DEFAULT, packetId)->createPacket(dataStream, this->connectionData);
                    int readBytes = (int) (p1->getStream().getPos() - previousPosition);
                    if (readBytes != packetSize) printf("Received packet [%d] contains unread data (%d)\n", packetId, packetSize - readBytes);
                    if (packetSize - readBytes < 0) throw std::runtime_error("Packet read more bytes than expected");
                    dataStream.skipBytes(packetSize - readBytes);
                } catch (const std::out_of_range &e) {
                    fprintf(stderr, "%s\n", e.what());
                }
            }
        }
    }

    template<class TPacket, typename... Args>
    void sendPacket(Args &&... args) {
        DataStream stream;
        stream.write((byte) TPacket::ID);
        stream.write((byte) 0);
        TPacket packet(stream, std::forward<Args>(args)...);
        packet.prepareData();
        auto &firstChunkPtr = *packet.getStream().getData().begin();
        firstChunkPtr[1] = (byte) (packet.getStream().getLength() - 2);
        auto &data = packet.getStream().getData();
        for (auto it = data.begin(); it != data.end(); ++it) {
            DataStream::DataChunk &chunk = *it;
            connectionData.cipher.encrypt(chunk.data(), std::min(packet.getStream().getLength(), (size_t) DataStream::MAX_LENGTH));
        }
        packet.getStream().seal();
        sendPacket(packet);
    }

    void sendPacket(const Packet &packet) {
        if (!packet.getStream().isSealed()) throw std::runtime_error("Trying to send an unsealed packet");
        printf("Tring to send data [%d] = ", packet.getStream().getLength());
        for (int i = 0; i < packet.getStream().getLength(); ++i) {
            printf("%d ", (int) packet.getStream().getByteAt(i));
        }
        printf("\n");
        auto &data = packet.getStream().getData();
        for (auto it = data.begin(); it != data.end(); ++it) {
            const DataStream::DataChunk &chunk = *it;
            ssize_t bytes_sent = send(sock, chunk.data(), std::min(packet.getStream().getLength(), (size_t) DataStream::MAX_LENGTH), 0);
            if (bytes_sent < 0) fprintf(stderr, "Failed to send packet\n");
            else if (bytes_sent != packet.getStream().getLength()) fprintf(stderr, "Sent packet with an unexpected size\n");
        }
    }

    ConnectionData connectionData;

private:
    const std::string hostAddress;
    const uint16_t port;
    hostent *host; //memory is handled internally by the socket library
    sockaddr_in serverAddress;
    int sock;
    bool connected = false;
};

#endif //PWI_OOG_NETWORKMANAGER_H
