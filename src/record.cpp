#include <tinystore/record.hpp>

#include <cstdint>

#include <tinystore/byte_codec.hpp>
#include <tinystore/limits.hpp>

namespace tinystore {

// PutMutation Layout:
// u8        Operation::PUT
// u64       LSN             little-endian
// u16       key length      little-endian
// u32       value length    little-endian
// variable  key content
// variable  value content
//
// @todo: key/value type?

void WriteAheadLogSerializer::serialize_mutation_type(Operation op) {
    ByteWriter bw(_stream);
    bw.write_u8(static_cast<uint8_t>(op));
}

std::expected<void, EncodeError>
WriteAheadLogSerializer::serialize_mutation(const PutMutation &pm) {
    if (pm.key.length() > limits::max_key_length)
        return std::unexpected(
            EncodeError{EncodeError::FieldTooLong{.field_name = "key",
                                                  .input_length = pm.key.length(),
                                                  .max_length = limits::max_key_length}});
    if (pm.value.length() > limits::max_value_length)
        return std::unexpected(
            EncodeError{EncodeError::FieldTooLong{.field_name = "value",
                                                  .input_length = pm.value.length(),
                                                  .max_length = limits::max_value_length}});

    const auto key_length = static_cast<uint16_t>(pm.key.length());
    const auto value_length = static_cast<uint32_t>(pm.value.length());

    std::size_t overall_length = sizeof(uint8_t)    // op
                                 + sizeof(uint16_t) // key length
                                 + sizeof(uint32_t) // val length
                                 + key_length + value_length;

    // @todo: this check should happen before we add up everything by subtracting from max
    if (overall_length <= key_length || overall_length <= value_length)
        return std::unexpected(EncodeError{EncodeError::Overflow()});

    serialize_mutation_type(Operation::PUT);

    ByteWriter bw(_stream);
    bw.write_u64_le(pm.lsn);
    bw.write_u16_le(key_length);
    bw.write_u32_le(value_length);
    bw.write_string(pm.key);
    bw.write_string(pm.value);

    return {};
}

[[nodiscard]] std::expected<void, EncodeError> WriteAheadLog::log_put(const PutMutation &pm) {
    const auto serialize_result = _serializer.serialize_mutation(pm);
    if (!serialize_result)
        return std::unexpected(serialize_result.error());
    return {};
}

} // namespace tinystore
