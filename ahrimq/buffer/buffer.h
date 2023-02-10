#ifndef _AHRIMQ_BUFFER_H_
#define _AHRIMQ_BUFFER_H_

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "base/nocopyable.h"

namespace ahrimq {

static const char *CRLF = "\r\n";

/// @brief Buffer defines a char buffer that grows automatically.
class Buffer : public NoCopyable {
 public:
  /// @brief Construct a new Buffer object.
  /// @param init_bufsize initial size of new buffer
  explicit Buffer(size_t init_bufsize = 1024);

  ~Buffer();

  /// @brief Get the capacity of buffer.
  /// @return
  inline size_t Capacity() const {
    return capacity_;
  }

  /**
   * @brief Get the index of reader.
   *
   * @return size_t
   */
  inline size_t BeginRead() const {
    return p_reader_;
  }

  /// @brief Get the writer index.
  /// @return
  inline size_t BeginWrite() const {
    return p_writer_;
  }

  /// @brief Get the number of writable bytes.
  /// @return
  inline size_t WritableBytes() const {
    return Capacity() - p_writer_;
  }

  /// @brief Get the number of readable bytes.
  /// @return
  inline size_t ReadableBytes() const {
    return p_writer_ - p_reader_;
  }

  /// @brief Return the size of buffer(readable bytes), this method is recommended,
  /// because its name is more comprehensible.
  /// @return
  inline size_t Size() const {
    return ReadableBytes();
  }

  inline bool Empty() const {
    return p_writer_ == p_reader_;
  }

  /// @brief Get the number of prependable bytes.
  /// @return
  inline size_t PrependableBytes() const {
    return p_reader_;
  }

  /// @brief Get the pointer of array underneath buffer.
  /// @return
  inline char *BufferFront() {
    return data_;
  }

  /// @brief Return readable bytes as std::string.
  /// @return
  std::string ReadableAsString() const {
    return std::string(data_ + p_reader_, ReadableBytes());
  }

  /// @brief Return readable as std::string.
  /// @return 
  std::string Content() const {
    return std::move(ReadableAsString());
  }

  /// @brief Get the pointer of the begin of readable bytes.
  /// @return
  inline const char *BeginReadPointer() const {
    return data_ + p_reader_;
  }

  /// @brief Get the pointer of the begin of writable bytes.
  /// @return
  inline char *BeginWritePointer() {
    return data_ + p_writer_;
  }

  /// @brief Append data from another buffer.
  /// @param other
  void Append(const Buffer &other);

  /// @brief Append more value into buffer.
  /// @param value
  void Append(const std::string &value);

  /// @brief Append more value into buffer.
  /// @param value
  /// @param len
  void Append(const char *value, int len);

  /// @brief Return the index of CRLF in readable bytes.
  /// @return
  int FindCRLFInReadable();

  /// @brief Reset the buffer.
  void Reset();

  /// @brief Move reader pointer to the right.
  /// @param len
  void ReaderIdxForward(size_t len);

  /// @brief Move reader pointer to the left.
  /// @param len
  void ReaderIdxBackward(size_t len);

  /// @brief Move writer pointer to the right.
  /// @param len
  void WriterIdxForward(size_t len);

  /// @brief Move writer pointer to the left.
  /// @param len
  void WriterIdxBackward(size_t len);

  /// @brief Return a string.
  /// @param len
  /// @return
  std::string ReadStringAndForward(size_t len);

  /// @brief Read a string from buffer.
  /// @param len
  /// @return
  std::string ReadString(size_t len);

  /// @brief Read string starting from index.
  /// @param index
  /// @param len
  /// @return
  std::string ReadStringFrom(size_t index, size_t len);

  /// @brief Get the char at index in readable bytes.
  /// @param index
  /// @return
  char ReadableCharacterAt(size_t index) const;

  /// @brief Read string from buffer and forward the pointer till delim.
  /// @param delim
  /// @return
  std::string ReadStringAndForwardTill(const char *delim = "\r\n");

  /// @brief Read string from buffer and forward the pointer till delim.
  /// @param delim the delimitor
  /// @param found output arg, if delim is found in buffer
  /// @return
  std::string ReadStringAndForwardTill(const char *delim, bool &found);

  /// @brief Consume all bytes in readable.
  /// @return
  std::vector<char> ReadAll();

  /// @brief Consume all bytes in readable.
  /// @return
  std::string ReadAllAsString();

  /// @brief Read a long integer from buffer and forward the pointer.
  /// @param step
  /// @return
  long ReadLongAndForward(size_t &step);

  /// @brief Read a long integer but do not forward a pointer.
  /// @param step
  /// @return
  long ReadLong(size_t &step);

  /// @brief Read a long starting from index.
  /// @param index
  /// @param step
  /// @return
  long ReadLongFrom(size_t index, size_t &step);

  /// @brief Stream buffer to output file.
  /// @param file
  void ToFile(const std::string &file);

  /// @brief Remove '\t', '\n', '\r', ' ' on the left side.
  void TrimLeft();

  /// @brief Remove '\t', '\n', '\r', ' ' on the right side.
  void TrimRight();

  /// @brief Remove '\t', '\n', '\r', ' ' on both side.
  void TrimLeftRight();

  /// @brief Make sure there is enough room.
  /// @param n
  void EnsureBytesForWrite(size_t n);

 private:
  /// @brief Move readable bytes to the head of buffer.
  void MoveReadableToHead();

 private:
  char* data_ = nullptr;
  size_t p_reader_ = 0;
  size_t p_writer_ = 0;
  size_t capacity_ = 0;
};

}  // namespace ahrimq

#endif  // _AHRIMQ_BUFFER_H_