//
// Created by dar on 6/15/16.
//

#include "RC4.h"
#include "Unpacker.h"
#include "DataStream.h"

#ifndef PWI_OOG_CIPHER_H
#define PWI_OOG_CIPHER_H

class Cipher {
public:
    void init(const std::vector<byte> &RC4Encode, const std::vector<byte> &RC4Decode) {
        encoder.shuffle(RC4Encode);
        decoder.shuffle(RC4Decode);
    }

    void encrypt(byte *packet, ssize_t bytes) {
        if (!encoder.isShuffled()) return;
        for (int i = 0; i < bytes; ++i)
            encoder.encode(packet[i]);
    }

    void decrypt(byte *packet, ssize_t &bytes) {
        if (!decoder.isShuffled()) return;
        for (int i = 0; i < bytes; ++i) decoder.encode(packet[i]);
        unpacker.Unpack(packet, bytes);
    }

private:
    RC4 encoder;
    RC4 decoder;
    Unpacker unpacker;
};

#endif //PWI_OOG_CIPHER_H
