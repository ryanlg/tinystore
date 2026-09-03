#include <tinystore/record.hpp>

#include <cstdint>
#include <utility>

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
        return std::unexpected(
            EncodeError{EncodeError::FieldTooLong{.field_name = "key",
                                                  .input_length = record.key.length(),
                                                  .max_length = limits::max_key_length}});
    if (record.value.length() > limits::max_value_length)
        return std::unexpected(
            EncodeError{EncodeError::FieldTooLong{.field_name = "value",
                                                  .input_length = record.value.length(),
                                                  .max_length = limits::max_value_length}});

    const auto key_length = static_cast<uint16_t>(record.key.length());
    const auto value_length = static_cast<uint32_t>(record.value.length());

    std::size_t overall_length = sizeof(uint8_t)    // op
                                 + sizeof(uint16_t) // key length
                                 + sizeof(uint32_t) // val length
                                 + key_length + value_length;

    // @todo: this check should happen before we add up everything by subtracting from max
    if (overall_length <= key_length || overall_length <= value_length)
        return std::unexpected(EncodeError{EncodeError::Overflow()});

    ByteWriter bw = {overall_length};
    bw.write_u8(static_cast<uint8_t>(record.op));
    bw.write_u16_le(key_length);
    bw.write_u32_le(value_length);
    bw.write_string(record.key);
    bw.write_string(record.value);

    return std::move(bw).finalize();
}

std::expected<Record, DecodeError> decode_record(ByteReader &br) {
    Record record = {};

    {
        const auto result = br.read_u8();
        if (!result)
            return std::unexpected(result.error());
        record.op = static_cast<Operation>(*result);

        if (record.op >= Operation::NUM_TYPES)
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

    const auto result = br.end_consumed();
    if (!result)
        return std::unexpected(result.error());

    return record;
}

std::expected<Record, DecodeError> decode_record(const std::vector<std::byte> &bvec,
                                                 const std::size_t starting_offset) {
    auto br = ByteReader(std::span(bvec), starting_offset);
    return decode_record(br);
}

void LogBatch::append(const Record &record) { _records.push_back(record); }

// LogBatch layout
// u64        lsn                  little endian
// u16        number_of_records    little endian
// variable   records
//
// @todo: u32        CRC       little endian
// @todo: sizes of the records are not limited, which means it's possible
//        to overflow. We can add this limit in when we consider adding
//        key/value types.
[[nodiscard]]
std::expected<std::vector<std::byte>, EncodeError> encode_batch(const LogBatch &batch) {
    if (batch.length() > limits::max_batch_length)
        return std::unexpected(
            EncodeError{EncodeError::FieldTooLong{.field_name = "number_of_records",
                                                  .input_length = batch.length(),
                                                  .max_length = limits::max_batch_length}});

    const auto batch_length = static_cast<uint16_t>(batch.length());
    const std::size_t header_length = sizeof(uint64_t)    // lsn
                                      + sizeof(uint16_t); // number_of_records

    ByteWriter bw = {header_length};
    bw.write_u64_le(batch.lsn());
    bw.write_u16_le(batch_length);

    for (const auto &record : batch.records()) {
        const auto encode_result = encode_record(record);
        if (!encode_result)
            return std::unexpected(encode_result.error());

        bw.write_span({encode_result->begin(), encode_result->end()});
    }

    return std::move(bw).finalize();
}

[[nodiscard]]
std::expected<LogBatch, DecodeError> decode_batch(const std::vector<std::byte> &bvec,
                                                  const std::size_t starting_offset) {
    std::vector<Record> recvec;
    ByteReader br(std::span(bvec), starting_offset);

    lsn_t lsn;
    {
        const auto result = br.read_u64_le();
        if (!result)
            return std::unexpected(result.error());
        lsn = *result;
    }

    // @todo: there is actually no reason to save this
    // Record deserialization doesn't rely on this field and is mostly self-governing.
    uint16_t number_of_records;
    {
        const auto result = br.read_u16_le();
        if (!result)
            return std::unexpected(result.error());
        number_of_records = *result;
    }
    recvec.reserve(number_of_records);

    while (!br.has_ended()) {
        const auto result = decode_record(br);
        if (!result)
            return std::unexpected(result.error());
        recvec.push_back(*result);
    }

    return LogBatch(lsn, std::move(recvec));
}

[[nodiscard]] std::expected<void, EncodeError>
WriteAheadLogWriter::append_put(const Record &record) {
    const auto encode_result = encode_record(record);
    if (!encode_result)
        return std::unexpected(encode_result.error());

    // @todo: a better way is to make Record take a ByteWriter and directly write to it.
    bw.write_span(*encode_result);
    return {};
}

} // namespace tinystore
