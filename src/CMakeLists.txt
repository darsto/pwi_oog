cmake_minimum_required(VERSION 3.5)
project(pwi_oog)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES main.cpp RC4.h Unpacker.h DataStream.h md5.cpp md5.h hmac.h ConnectionData.h Cipher.h packet/Packet.h packet/client/LoginPacket.h HexConverter.h HexConverter.cpp packet/client/CMKeyPacket.h ConnectionManager.cpp ConnectionManager.h packet/PacketManager.h packet/server/ServerInfoPacket.h packet/server/SMKeyPacket.h packet/PacketManager.cpp packet/ServerPacketFactory.h packet/server/OnlineAnnouncePacket.h packet/client/RoleListRequestPacket.h packet/client/KeepAlivePacket.h packet/server/LastLoginInfoPacket.h packet/server/PingPacket.h packet/server/RoleListPacket.h DataStream.cpp packet/client/SelectRolePacket.h pwi/RoleInfo.h packet/client/EnterWorldPacket.h packet/server/SelectRoleConfirmationPacket.h packet/server/PrivateMessageInPacket.h)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} profiler pthread)