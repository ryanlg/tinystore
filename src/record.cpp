#include <tinystore/record.hpp>

#include <cstdint>
#include <utility>

#include <tinystore/byte_codec.hpp>
#include <tinystore/limits.hpp>

namespace tinystore {

// Record Layout:
// u8        operation
// u16       key length      little-endian
// u32       value length    little-endian
// variable  key content
// variable  value content
//
// @todo: key/value type?

std::expected<std::vector<std::byte>, EncodeError> encode_record(const Record &record) {
    if (record.key.length() > limits::max_key_length)
        return std::unexpected(EncodeError::field_too_long("key"));
    if (record.value.length() > limits::max_value_length)
        return std::unexpected(EncodeError::field_too_long("value"));

    const auto key_length = static_cast<uint16_t>(record.key.length());
    const auto value_length = static_cast<uint32_t>(record.value.length());

    std::size_t overall_length = sizeof(uint8_t)    // op
                                 + sizeof(uint16_t) // key length
                                 + sizeof(uint32_t) // val length
                                 + key_length + value_length;

    // @todo: this check should happen before we add up everything by subtracting from max
    if (overall_length <= key_length || overall_length <= value_length)
        return std::unexpected(EncodeError::overflow());

    ByteWriter bw = {overall_length};
    bw.write_u8(static_cast<uint8_t>(record.op));
    bw.write_u16_le(key_length);
    bw.write_u32_le(value_length);
    bw.write_string(record.key);
    bw.write_string(record.value);

    return std::move(bw).finalize();
}

std::expected<Record, DecodeError> decode_record(const std::vector<std::byte> &bvec) {
    Record record = {};
    auto br = ByteReader(std::span(bvec));

    {
        const auto result = br.read_u8();
        if (!result)
            return std::unexpected(result.error());
        record.op = static_cast<Operation>(*result);

        if (record.op >= Operation::NUM_TYPES)
            return std::unexpected(DecodeError::invalid_value());
    }

    uint16_t key_len;
    {
        const auto result = br.read_u16_le();
        if (!result)
            return std::unexpected(result.error());
        key_len = *result;
    }

    uint32_t value_len;
    {
        const auto result = br.read_u32_le();
        if (!result)
            return std::unexpected(result.error());
        value_len = *result;
    }

    {
        const auto result = br.read_string(key_len);
        if (!result)
            return std::unexpected(result.error());
        record.key = *result;
    }

    {
        const auto result = br.read_string(value_len);
        if (!result)
            return std::unexpected(result.error());
        record.value = *result;
    }

    if (const auto result = br.end_consumed())
        return std::unexpected(*result);

    return record;
}

} // namespace tinystore
