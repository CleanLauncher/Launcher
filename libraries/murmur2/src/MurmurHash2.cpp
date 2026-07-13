
#include "MurmurHash2.h"

namespace Murmur2 {

const uint32_t m = 0x5bd1e995;
const int r = 24;

uint32_t hash(Reader* file_stream, std::size_t buffer_size, std::function<bool(char)> filter_out)
{
    auto* buffer = new char[buffer_size];
    char data[4];

    int read = 0;
    uint32_t size = 0;

    do {
        read = file_stream->read(buffer, buffer_size);
        for (int i = 0; i < read; i++) {
            if (!filter_out(buffer[i]))
                size += 1;
        }
    } while (!file_stream->eof());

    file_stream->goToBeginning();

    int index = 0;

    IncrementalHashInfo info{ (uint32_t)1 ^ size, (uint32_t)size };
    do {
        read = file_stream->read(buffer, buffer_size);
        for (int i = 0; i < read; i++) {
            char c = buffer[i];

            if (filter_out(c))
                continue;

            data[index] = c;
            index = (index + 1) % 4;

            if (index == 0)
                FourBytes_MurmurHash2(reinterpret_cast<unsigned char*>(&data), info);
        }
    } while (!file_stream->eof());

    FourBytes_MurmurHash2(reinterpret_cast<unsigned char*>(&data), info);

    delete[] buffer;

    return info.h;
}

void FourBytes_MurmurHash2(const unsigned char* data, IncrementalHashInfo& prev)
{
    if (prev.len >= 4) {

        uint32_t k = *reinterpret_cast<const uint32_t*>(data);

        k *= m;
        k ^= k >> r;
        k *= m;

        prev.h *= m;
        prev.h ^= k;

        prev.len -= 4;
    } else {

        switch (prev.len) {
            case 3:
                prev.h ^= data[2] << 16;

            case 2:
                prev.h ^= data[1] << 8;

            case 1:
                prev.h ^= data[0];
                prev.h *= m;
        };

        prev.h ^= prev.h >> 13;
        prev.h *= m;
        prev.h ^= prev.h >> 15;

        prev.len = 0;
    }
}

}
