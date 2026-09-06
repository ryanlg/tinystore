#ifndef TINYSTORE_RECORD_HPP
#define TINYSTORE_RECORD_HPP

#include <tinystore/error.hpp>
#include <tinystore/mutation.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <vector>

namespace tinystore {

enum class Operation : uint8_t {
    PUT = 1,
    NUM_TYPES,
};

class WriteAheadLogSerializer {
  public:
    WriteAheadLogSerializer(std::vector<std::byte> &stream) : _stream(stream) {}

    std::expected<void, EncodeError> serialize_mutation(const PutMutation &);

  private:
    std::vector<std::byte> &_stream;
    void serialize_mutation_type(Operation);
};

class WriteAheadLog {
  public:
    WriteAheadLog() : _serializer(WriteAheadLogSerializer(_stream)) {}

    [[nodiscard]] std::expected<void, EncodeError> log_put(const PutMutation &);

  private:
    std::vector<std::byte> _stream;
    WriteAheadLogSerializer _serializer;
};

} // namespace tinystore

#endif // TINYSTORE_RECORD_HPP
