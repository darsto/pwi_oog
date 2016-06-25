//
// Created by dar on 6/9/16.
//

#ifndef PWI_OOG_RC4_H
#define PWI_OOG_RC4_H

#include <vector>
#include "DataStream.h"

class RC4 {
private:
    byte shift1 = 0;
    byte shift2 = 0;
    byte table[256];
    bool shuffled = false;

public:
    void encode(byte &bt) {
        shift1++;
        shift2 += table[shift1];

        byte a = table[shift1];
        byte b = table[shift2];

        table[shift2] = a;
        table[shift1] = b;

        byte d = table[(byte) (a + b)];

        bt ^= d;
    }

    void shuffle(const std::vector<byte> &key) {
        for (int i = 0; i < 256; i++) {
            table[i] = (byte) i;
        }

        int shift = 0;

        for (int i = 0; i < 256; i++) {
            shift += key[i % key.size()] + table[i];
            shift %= 256;

            byte a = table[i];
            byte b = table[shift];

            table[i] = b;
            table[shift] = a;
        }

        shuffled = true;
    }

    bool isShuffled() const {
        return shuffled;
    }
};

#endif //PWI_OOG_RC4_H
