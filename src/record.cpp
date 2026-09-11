#include <tinystore/record.hpp>

#include <cerrno>
#include <cstdint>
#include <stdexcept>
#include <utility>

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
    WriteAheadLogSerializer serializer(_stream);
    const auto serialize_result = serializer.serialize_mutation(pm);
    if (!serialize_result)
        return std::unexpected(serialize_result.error());

    _wal_file.append({_stream.begin(), _stream.end()});
    _wal_file.sync();
    _stream.clear();

    return {};
}

BufferedFileAppender::BufferedFileAppender(std::filesystem::path path) {
    // @todo: exceptions and close on failure
    _fd = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (_fd == -1)
        throw std::runtime_error("failed to open wal");
}

BufferedFileAppender::~BufferedFileAppender() {
    if (_fd != -1)
        close(_fd);
}

BufferedFileAppender::BufferedFileAppender(BufferedFileAppender &&other) noexcept
    : _fd(std::exchange(other._fd, -1)), _buffer(std::move(other._buffer)) {}

BufferedFileAppender &BufferedFileAppender::operator=(BufferedFileAppender &&other) noexcept {
    if (this != &other) {
        if (_fd != -1)
            close(_fd);
        _fd = std::exchange(other._fd, -1);
        _buffer = std::move(other._buffer);
    }
    return *this;
}

void BufferedFileAppender::append(std::span<std::byte> data) {
    _buffer.insert(_buffer.end(), data.begin(), data.end());
}

void BufferedFileAppender::sync() {
    std::size_t written = 0;
    while (written < _buffer.size()) {
        const ssize_t rc = write(_fd, _buffer.data() + written, _buffer.size() - written);
        if (rc == -1) {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("failed to write wal");
        }
        written += static_cast<std::size_t>(rc);
    }

#if defined(__DARWIN__) || defined(__APPLE__)
    // fsync() on macOS doesn't make the drive flush its own cache, F_FULLFSYNC does.
    if (fcntl(_fd, F_FULLFSYNC) == -1)
        throw std::runtime_error("failed to fcntl F_FULLFSYNC");
#else
    if (fsync(_fd) == -1)
        throw std::runtime_error("failed to fsync wal");
#endif
    _buffer.clear();
}

} // namespace tinystore
