#include <vector>
#include "DataStream.h"

class Unpacker {
private:
    int code1 = 0;
    int code2 = 0;
    int code3 = 0;
    int code4 = 0;

    byte packedOffset = 0;

    std::vector<byte> packedBytes;
    std::vector<byte> unpackedBytes;
    std::vector<byte> unpackedChunk;

public:
    Unpacker() : unpackedBytes(10500) {} //TODO doesn't work without this - there are probably some reallocation (deallocation) issues

    std::vector<byte> Unpack(byte packedByte) {
        packedBytes.push_back(packedByte);

        unpackedChunk.clear();

        if (unpackedBytes.size() >= 10240)
            unpackedBytes.erase(unpackedBytes.begin() + 0, unpackedBytes.begin() + 2048);

        for (;;) {
            if (code3 == 0) {
                if (HasBits(4)) {
                    if (GetPackedBits(1) == 0) {
                        // 0-xxxxxxx
                        code1 = 1;
                        code3 = 1;
                    }
                    else {
                        if (GetPackedBits(1) == 0) {
                            // 10-xxxxxxx
                            code1 = 2;
                            code3 = 1;
                        }
                        else {
                            if (GetPackedBits(1) == 0) {
                                // 110-xxxxxxxxxxxxx-*
                                code1 = 3;
                                code3 = 1;
                            }
                            else {
                                if (GetPackedBits(1) == 0) {
                                    // 1110-xxxxxxxx-*
                                    code1 = 4;
                                    code3 = 1;
                                }
                                else {
                                    // 1111-xxxxxx-*
                                    code1 = 5;
                                    code3 = 1;
                                }
                            }
                        }
                    }
                }
                else
                    break;
            }
            else if (code3 == 1) {
                if (code1 == 1) {
                    if (HasBits(7)) {
                        byte outB = (byte) GetPackedBits(7);
                        unpackedChunk.push_back(outB);
                        unpackedBytes.push_back(outB);
                        code3 = 0;
                    }
                    else
                        break;
                }
                else if (code1 == 2) {
                    if (HasBits(7)) {
                        byte outB = (byte) (GetPackedBits(7) | 0x80);
                        unpackedChunk.push_back(outB);
                        unpackedBytes.push_back(outB);
                        code3 = 0;
                    }
                    else
                        break;
                }
                else if (code1 == 3) {
                    if (HasBits(13)) {
                        code4 = (int) GetPackedBits(13) + 0x140;
                        code3 = 2;
                    }
                    else
                        break;
                }
                else if (code1 == 4) {
                    if (HasBits(8)) {
                        code4 = (int) GetPackedBits(8) + 0x40;
                        code3 = 2;
                    }
                    else
                        break;
                }
                else if (code1 == 5) {
                    if (HasBits(6)) {
                        code4 = (int) GetPackedBits(6);
                        code3 = 2;
                    }
                    else
                        break;
                }
            }
            else if (code3 == 2) {
                if (code4 == 0) {
                    // Guess !!!
                    if (packedOffset != 0) {
                        packedOffset = 0;
                        packedBytes.erase(packedBytes.begin() + 0);
                    }
                    code3 = 0;
                    continue;
                }
                code2 = 0;
                code3 = 3;
            }
            else if (code3 == 3) {
                if (HasBits(1)) {
                    if (GetPackedBits(1) == 0) {
                        code3 = 4;
                    }
                    else {
                        code2++;
                    }
                }
                else
                    break;
            }
            else if (code3 == 4) {
                int copySize;

                if (code2 == 0) {
                    copySize = 3;
                }
                else {
                    int size = code2 + 1;

                    if (HasBits(size)) {
                        copySize = (int) GetPackedBits(size) + (1 << size);
                    }
                    else
                        break;
                }

                Copy(code4, copySize);
                code3 = 0;
            }
        }

        return unpackedChunk;
    }

    size_t Unpack(DataStream &stream, const DataStream::DataChunk &compressedBytes, size_t size) {
        size_t offset = 0;
        for (int i = 0; i < size; ++i) {
            std::vector<byte> unpacked = Unpack(compressedBytes[i]);
            for (int j = 0; j < unpacked.size(); ++j) {
                stream.write(unpacked[j]);
            }
            offset += unpacked.size();
        }
        return offset;
    }

private:
    void Copy(int shift, int size) {
        for (int i = 0; i < size; i++) {
            int pIndex = (int)unpackedBytes.size() - shift;

            if (pIndex < 0)
                return;

            byte b = unpackedBytes.at(pIndex);
            unpackedBytes.push_back(b);
            unpackedChunk.push_back(b);
        }
    }

    uint GetPackedBits(int bitCount) {
        if (bitCount > 16)
            return 0;

        if (!HasBits(bitCount))
            throw new std::runtime_error("Unpack bit stream overflow");

        int alBitCount = bitCount + packedOffset;
        int alByteCount = (alBitCount + 7) / 8;

        uint v = 0;

        for (int i = 0; i < alByteCount; i++) {
            v |= (uint) (packedBytes[i]) << (24 - i * 8);
        }

        v <<= packedOffset;
        v >>= 32 - bitCount;

        packedOffset += (byte) bitCount;
        int freeBytes = packedOffset / 8;

        if (freeBytes > 0 && packedBytes.size() > 0)
            packedBytes.erase(packedBytes.begin() + 0, packedBytes.begin() + std::min(freeBytes, (int)packedBytes.size()));

        packedOffset %= 8;
        return v;
    }

    bool HasBits(int count) {
        return ((int)packedBytes.size() * 8 - packedOffset) >= count;
    }
};
