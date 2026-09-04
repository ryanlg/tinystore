#ifndef TINYSTORE_RECORD_HPP
#define TINYSTORE_RECORD_HPP

#include <tinystore/byte_codec.hpp>
#include <tinystore/error.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <utility>

namespace tinystore {

enum class Operation : uint8_t {
    PUT = 1,
    NUM_TYPES,
};

class PutRecord {
  public:
    static constexpr const Operation op = Operation::PUT;

    PutRecord(std::string key, std::string value)
        : _key(std::move(key)), _value(std::move(value)) {}
    ~PutRecord() = default;

    [[nodiscard]] const std::string key() const { return _key; };
    [[nodiscard]] const std::string value() const { return _value; };

    [[nodiscard]] std::expected<void, EncodeError> serialize(ByteWriter &bw) const;

    // @design:
    // The general design of returning a Record isn't great.
    // The WAL replayer should be consuming this Record and replaying it immediately. So all the
    // effort here copying the data from ByteReader's buffer is wasted.
    [[nodiscard]] static std::expected<PutRecord, DecodeError> deserialize(ByteReader &br);

  private:
    std::string _key;
    std::string _value;
};

class WriteAheadLog {
  public:
    [[nodiscard]] std::expected<void, EncodeError> log_put(const PutRecord &);

  private:
    ByteWriter bw;
};

} // namespace tinystore

#endif // TINYSTORE_RECORD_HPP
