#include <cstdint>
#include <iostream>
#include <utility>

#include <tinystore/byte_writer.hpp>
#include <tinystore/record.hpp>

namespace tinystore {

std::vector<std::byte> encode_record(const Record &record) {
    unsigned int overall_length = 1   // op
                                  + 4 // key_len
                                  + 4 // val_len
                                  + record.key.length() + record.value.length();

    ByteWriter bw = {overall_length};
    bw.write(static_cast<uint8_t>(record.op));
    // @todo: this is 8 bytes long
    bw.write(record.key.length());
    bw.write(record.value.length());
    bw.write(record.key);
    bw.write(record.value);

    return bw.finalize();
}

Record decode_record(const std::vector<std::byte> &bvec) {
    Record record = {};
    const auto data = bvec.data();
    off_t offset = 0;

    record.op = static_cast<Operation>(*reinterpret_cast<const uint8_t *>(data));
    offset += sizeof(uint8_t);

    size_t key_len = *reinterpret_cast<const size_t *>(data + offset);
    offset += sizeof(size_t);

    size_t value_len = *reinterpret_cast<const size_t *>(data + offset);
    offset += sizeof(size_t);

    record.key = std::string(reinterpret_cast<const char *>(data + offset), key_len);
    offset += key_len;

    record.value = std::string(reinterpret_cast<const char *>(data + offset), value_len);

    return record;
}

} // namespace tinystore
