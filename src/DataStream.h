//
// Created by dar on 6/10/16.
//

#ifndef PWI_OOG_DATASTREAM_H
#define PWI_OOG_DATASTREAM_H

#include <sys/types.h>
#include <cstdio>
#include <exception>
#include <vector>
#include <array>
#include <memory>
#include <list>

using byte = unsigned char;

class DataStream;

struct uni_int { //universal int, takes 1, 2, 4 or 5 bytes
    int32_t value;

    uni_int();
    uni_int(int32_t value);
    operator int32_t();
    void read(DataStream &stream);
    void write(DataStream &stream);
};

/*
 * A wrapper class for serialized data.
 */
class DataStream {
public:
    static const int MAX_LENGTH = 1024;
    using DataChunk = std::array<byte, MAX_LENGTH>;

private:
    template<uint N, typename T>
    struct has_bytes {
        static constexpr bool value = sizeof(T) >= N;
    };

    template<typename T>
    struct can_read {
    private:
        static constexpr auto check()
        -> typename std::is_same<
            void, decltype(std::declval<T>().read(std::declval<DataStream &>()))
        >::type;

        template<typename>
        static constexpr std::false_type check(...);

        typedef decltype(check()) type;

    public:
        static constexpr bool value = type::value;
    };

    template<typename T>
    struct can_write {
    private:
        static constexpr auto check()
        -> typename std::is_same<
            void, decltype(std::declval<T>().write(std::declval<DataStream &>()))
        >::type;

        template<typename>
        static constexpr std::false_type check(...);

        typedef decltype(check()) type;

    public:
        static constexpr bool value = type::value;
    };

    using Data = std::list<DataChunk>;

public:
    DataStream()
        : length(0),
          offset(0),
          pos(0),
          sealed(false),
          swapped(true) {
        data.push_back(DataChunk());
        current = data.begin();
    }

    template<typename T>
    typename std::enable_if_t<std::is_fundamental<std::decay_t<T>>::value>
    read(T &t) {
        ByteData <T> bd(*this);
        t = std::move(bd.value);
    }

    template<typename T>
    typename std::enable_if_t<!std::is_fundamental<std::decay_t<T>>::value
                              &&
                              !std::is_same<std::decay_t<T>, std::string>::value>
    read(T &t) {
        static_assert(can_read<T>::value, "Trying to read an unserializable object");
        t.read(*this);
    }

    void read(std::string &str) {
        auto str_array = read<std::vector<byte>>();
        str = std::string(str_array.begin(), str_array.end());
    }

    template<typename T>
    void read(std::vector<T> &vec, size_t vec_length_t = 0) {
        uni_int vec_length = {(int32_t) vec_length_t};
        if (vec_length == 0) read(vec_length);
        //this is just an optimization. If T is a non-fundamental type, sizeof(T) might contain extra padding bytes of T, which are not read from the stream
        int t_size = std::is_fundamental<std::decay_t<T>>::value ? sizeof(T) : 1;
        if (this->getPos() + vec_length * t_size > this->getLength()) throw std::runtime_error("Buffer overflow");
        vec.resize(vec_length);
        for (int i = 0; i < vec_length; ++i) {
            read(vec[i]);
        }
    }

    template<typename T>
    T read() {
        T val;
        read(val);
        return val;
    }

    template<typename T>
    typename std::enable_if_t<std::is_fundamental<std::decay_t<T>>::value>
    write(T &&t) {
        ByteData <T> bd(*this, static_cast<T>(t));
    }

    template<typename T>
    typename std::enable_if_t<!std::is_fundamental<std::decay_t<T>>::value
                              &&
                              !std::is_same<std::decay_t<T>, std::string>::value>
    write(T &&t) {
        static_assert(can_write<T>::value, "Trying to write an unserializable object");
        t.write(*this);
    }

    template <typename T>
    typename std::enable_if_t<std::is_same<std::decay_t<T>, std::string>::value>
    write(T &&str) {
        std::vector<byte> data(str.begin(), str.end());
        write(data);
    }

    template<typename T>
    void write(const std::vector<T> &vec) {
        uni_int vec_length = {(int32_t) vec.size()};
        write(vec_length);
        for (const T &t : vec) {
            write(static_cast<T>(t));
        }
    }

    template<typename T>
    void write(std::vector<T> &vec) {
        uni_int vec_length = {(int32_t) vec.size()};
        write(vec_length);
        for (const T &t : vec) {
            write(static_cast<T>(t));
        }
    }

    const byte getByteAt(u_long index) const {
        if (index >= getLength()) throw std::out_of_range("Trying to read a byte at index > length");
        Data::const_iterator it = data.cbegin();
        while (index > MAX_LENGTH) {
            ++it;
            index -= MAX_LENGTH;
        }
        const DataChunk &chunkPtr = *it;
        return chunkPtr[index];
    }

    const Data &getData() const {
        return data;
    }

    Data &getData() {
        return data;
    }

    size_t getLength() const {
        return length;
    }

    u_long getPos() const {
        return pos;
    }

    bool isSealed() const {
        return sealed;
    }

    void seal() {
        sealed = true;
        current = data.begin();
        pos = 0;
    }

    bool isSwapped() const {
        return swapped;
    }

    void setSwapped(bool swapped) {
        this->swapped = swapped;
    }

    void skipBytes(size_t n) {
        incrementPos(n);
    }

private:
    Data data;
    Data::iterator current;
    size_t length;
    u_long pos;
    u_long offset;

    bool sealed; //when true, the stream is read-only

    /*when true, all primitives are construced from read bytes in reverse order
     * [1 1 0 0] -> (swapped == 1) (int) 257
     * true by default */
    bool swapped;

    byte &getNextByte() {
        auto prev_pos = pos - offset;
        DataChunk &chunkPtr = *current;
        incrementPos();
        return chunkPtr[prev_pos];
    }

    void incrementPos(u_long n = 1) {
        if (sealed && pos + n > length) throw std::out_of_range("pos + n > length");
        pos += n;
        if (!sealed) length = pos;
        while (pos - offset >= MAX_LENGTH) {
            offset += MAX_LENGTH;
            ++current;
            if (!sealed && current == data.end()) {
                appendChunk();
                --current;
            }
        }
    }

    void appendChunk() {
        if (sealed) throw std::runtime_error("Trying to..."); //todo
        data.push_back(DataChunk());
    }

    /*
     * A wrapper class for primitives (de)serialization
     */
    template<typename T>
    union ByteData {
        std::decay_t<T> value;
        byte bytes[sizeof(T)];

        static_assert(std::is_fundamental<std::decay_t<T>>::value, "Trying to create ByteData of non-fundamental type");

        ByteData(DataStream &stream) {
            if (stream.getPos() + sizeof(T) > stream.getLength()) throw std::runtime_error("Buffer overflow");
            for (int i = 0; i < sizeof(T); ++i) {
                bytes[stream.isSwapped() ? (sizeof(T) - 1 - i) : i] = stream.getNextByte();
            }
        }

        template<typename U = T>
        ByteData(DataStream &stream, U &&value) {
            if (stream.isSealed()) throw std::runtime_error("Trying to write bytes into a sealed stream.");
            this->value = value;
            for (int i = 0; i < sizeof(U); ++i) {
                stream.getNextByte() = bytes[stream.isSwapped() ? (sizeof(U) - 1 - i) : i];
            }
        }
    };
};

#endif //PWI_OOG_DATASTREAM_H
