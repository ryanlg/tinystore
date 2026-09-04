#include <tinystore/record.hpp>

#include <cstdint>
#include <utility>

#include <tinystore/limits.hpp>

namespace tinystore {

// PutRecord Layout:
// u8        operation
// u16       key length      little-endian
// u32       value length    little-endian
// variable  key content
// variable  value content
//
// @todo: key/value type?

std::expected<void, EncodeError> PutRecord::serialize(ByteWriter &bw) const {
    if (_key.length() > limits::max_key_length)
        return std::unexpected(
            EncodeError{EncodeError::FieldTooLong{.field_name = "key",
                                                  .input_length = _key.length(),
                                                  .max_length = limits::max_key_length}});
    if (_value.length() > limits::max_value_length)
        return std::unexpected(
            EncodeError{EncodeError::FieldTooLong{.field_name = "value",
                                                  .input_length = _value.length(),
                                                  .max_length = limits::max_value_length}});

    const auto key_length = static_cast<uint16_t>(_key.length());
    const auto value_length = static_cast<uint32_t>(_value.length());

    std::size_t overall_length = sizeof(uint8_t)    // op
                                 + sizeof(uint16_t) // key length
                                 + sizeof(uint32_t) // val length
                                 + key_length + value_length;

    // @todo: this check should happen before we add up everything by subtracting from max
    if (overall_length <= key_length || overall_length <= value_length)
        return std::unexpected(EncodeError{EncodeError::Overflow()});

    bw.write_u8(static_cast<uint8_t>(op));
    bw.write_u16_le(key_length);
    bw.write_u32_le(value_length);
    bw.write_string(_key);
    bw.write_string(_value);

    return {};
}

std::expected<PutRecord, DecodeError> PutRecord::deserialize(ByteReader &br) {
    Operation op;
    {
        const auto result = br.read_u8();
        if (!result)
            return std::unexpected(result.error());
        op = static_cast<Operation>(*result);

        if (op >= Operation::NUM_TYPES)
            return std::unexpected(DecodeError{DecodeError::InvalidValue{
                .field_name = "op",
            }});
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

    std::string key;
    {
        const auto result = br.read_string(key_len);
        if (!result)
            return std::unexpected(result.error());
        key = *result;
    }

    std::string value;
    {
        const auto result = br.read_string(value_len);
        if (!result)
            return std::unexpected(result.error());
        value = *result;
    }

    return PutRecord(std::move(key), std::move(value));
}

[[nodiscard]] std::expected<void, EncodeError> WriteAheadLog::log_put(const PutRecord &record) {
    const auto serialize_result = record.serialize(bw);
    if (!serialize_result)
        return std::unexpected(serialize_result.error());
    return {};
}

} // namespace tinystore
