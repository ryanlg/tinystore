#ifndef TINYSTORE_RECORD_HPP
#define TINYSTORE_RECORD_HPP

#include <tinystore/error.hpp>
#include <tinystore/mutation.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

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

class BufferedFileAppender {
  public:
    BufferedFileAppender(std::filesystem::path path);
    ~BufferedFileAppender();

    BufferedFileAppender(const BufferedFileAppender &) = delete;
    BufferedFileAppender &operator=(const BufferedFileAppender &) = delete;
    BufferedFileAppender(BufferedFileAppender &&other) noexcept;
    BufferedFileAppender &operator=(BufferedFileAppender &&other) noexcept;

    void append(std::span<std::byte> data);
    void sync();

  private:
    int _fd;
    std::vector<std::byte> _buffer;
};

class WriteAheadLog {
  public:
    WriteAheadLog(std::filesystem::path path) : _wal_file(BufferedFileAppender(path)) {}

    [[nodiscard]] std::expected<void, EncodeError> log_put(const PutMutation &);

  private:
    BufferedFileAppender _wal_file;
    std::vector<std::byte> _stream;
};

} // namespace tinystore

#endif // TINYSTORE_RECORD_HPP
