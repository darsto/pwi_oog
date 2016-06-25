//
// Created by dar on 6/16/16.
//

#include "HexConverter.h"

unsigned char hexToDec(char hex) {
    return (unsigned char) ((hex >= 'a') ? (hex - 'a' + 10) : (hex - '0'));
}

std::vector<byte> hexToDec(const std::string &hex) {
    size_t size = (hex.size() - 1) / 2 + 1;
    std::vector<byte> ret(size);
    for (int i = 0; i < size; ++i) {
        byte dec16 = hexToDec(hex.at(2 * i));
        byte dec1 = hexToDec(hex.at(2 * i + 1));
        ret[i] = (byte) ((16 * dec16) + dec1);
    }
    return ret;
}
