//
// Created by dar on 6/30/16.
//

#include "DataStream.h"

uni_int::uni_int(int32_t value) : value(value) {}

uni_int::operator int32_t() {
    return value;
}

void uni_int::read(DataStream &stream) {
    byte firstByte = stream.getByteAt(stream.getPos());
    switch (firstByte & 0xE0) {
        case 0xE0: {
            stream.skipBytes(1);
            stream.read(value); // 4 bytes
            break;
        }
        case 0xC0: {
            stream.read(value); // 4 bytes
            value &= 0x3FFFFFFF;
            break;
        }
        case 0x80:
        case 0xA0: {
            int16_t cast;
            stream.read(cast); // 2 bytes
            value = cast & 0x7FFF;
            break;
        }
        default: {
            byte cast;
            stream.read(cast); // 1 byte
            value = cast;
            break;
        }
    }
}

void uni_int::write(DataStream &stream) {
    if (value < 0x80) {
        stream.write((byte) value);
    } else if (value < 0x4000) {
        stream.write((int16_t) (value | 0x8000));
    } else if (value < 0x20000000) {
        stream.write(value | 0xC0000000);
    } else {
        stream.write((byte) 0xE0);
        stream.write(value);
    }
}
