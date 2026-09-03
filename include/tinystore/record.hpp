#ifndef TINYSTORE_RECORD_HPP
#define TINYSTORE_RECORD_HPP

#include <tinystore/byte_codec.hpp>
#include <tinystore/error.hpp>
#include <tinystore/types.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <utility>
#include <vector>

namespace tinystore {

enum class Operation : uint8_t {
    PUT = 1,
    DELETE = 2,
    NUM_TYPES,
};

struct Record {
    Operation op;
    std::string key;
    std::string value;
};

[[nodiscard]]
std::expected<std::vector<std::byte>, EncodeError> encode_record(const Record &);

[[nodiscard]]
std::expected<Record, DecodeError> decode_record(const std::vector<std::byte> &,
                                                 const std::size_t = 0);

/// A batch of write-ahead log, made up of multiple `Record`s.
/// This is the smallest unit that we can save, move around or replicate.
class LogBatch {
  public:
    LogBatch() = default;
    LogBatch(lsn_t lsn) : _lsn(lsn) {}
    LogBatch(lsn_t lsn, std::vector<Record> recvec) : _lsn(lsn), _records(std::move(recvec)) {}

    [[nodiscard]] const std::vector<Record> &records() const noexcept { return _records; }
    [[nodiscard]] std::size_t length() const { return _records.size(); }
    [[nodiscard]] lsn_t lsn() const noexcept { return _lsn; }

    void append(const Record &);

  private:
    lsn_t _lsn;
    std::vector<Record> _records;
};

[[nodiscard]]
std::expected<std::vector<std::byte>, EncodeError> encode_batch(const LogBatch &);

[[nodiscard]]
std::expected<LogBatch, DecodeError> decode_batch(const std::vector<std::byte> &,
                                                  const std::size_t = 0);

class WriteAheadLogWriter {
  public:
    [[nodiscard]] std::expected<void, EncodeError> append_put(const Record &);

  private:
    ByteWriter bw;
};

} // namespace tinystore

#endif // TINYSTORE_RECORD_HPP
