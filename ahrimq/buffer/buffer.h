#ifndef _BUFFER_H_
#define _BUFFER_H_

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
  /// @brief construct a new Buffer object
  /// @param init_bufsize initial size of new buffer
  explicit Buffer(size_t init_bufsize = 64)
      : data_(init_bufsize), p_reader_(0), p_writer_(0) {}

  ~Buffer() {
    data_.clear();
  }

  /// @brief get the capacity of buffer
  /// @return
  inline size_t Capacity() const {
    return data_.capacity();
  }

  /**
   * @brief get the index of reader
   *
   * @return size_t
   */
  inline size_t BeginReadIdx() const {
    return p_reader_;
  }

  /// @brief get the writer index
  /// @return
  inline size_t BeginWriteIdx() const {
    return p_writer_;
  }

  /// @brief get the number of writable bytes
  /// @return
  inline size_t WritableBytes() const {
    return Capacity() - p_writer_;
  }

  /// @brief get the number of readable bytes
  /// @return
  inline size_t ReadableBytes() const {
    return p_writer_ - p_reader_;
  }

  /// @brief get the number of prependable bytes
  /// @return
  inline size_t PrependableBytes() const {
    return p_reader_;
  }

  /// @brief get the pointer of array underneath buffer
  /// @return
  inline char *BufferFront() {
    return &data_[0];
  }

  /// @brief return readable bytes as std::string
  /// @return
  std::string ReadableAsString() const {
    return std::string(data_.data() + p_reader_, ReadableBytes());
  }

  /// @brief get the pointer of the begin of readable bytes
  /// @return
  inline char *BeginRead() {
    return data_.data() + p_reader_;
  }

  /// @brief get the pointer of the begin of writable bytes
  /// @return
  inline char *BeginWrite() {
    return data_.data() + p_writer_;
  }

  /// @brief append more value into buffer
  /// @param value
  void Append(const std::string &value);

  /// @brief append more value into buffer
  /// @param value
  /// @param len
  void Append(const char *value, int len);

  /// @brief return the index of CRLF in readable bytes
  /// @return
  int FindCRLFInReadable();

  /// @brief reset the buffer
  void Reset();

  /// @brief move pointer to the left
  /// @param len
  void ReaderIdxForward(size_t len);

  /// @brief move pointer to the right
  /// @param len
  void ReaderIdxBackward(size_t len);

  /// @brief return a string
  /// @param len
  /// @return
  std::string ReadStringAndForward(size_t len);

  /// @brief read a string from buffer
  /// @param len
  /// @return
  std::string ReadString(size_t len);

  /// @brief read string starting from index
  /// @param index
  /// @param len
  /// @return
  std::string ReadStringFrom(size_t index, size_t len);

  /// @brief get the char at index in readable bytes
  /// @param index
  /// @return
  char ReadableCharacterAt(size_t index) const;

  /// @brief read string from buffer and forward the pointer till delmi
  /// @param delim
  /// @return
  std::string ReadStringAndForwardTill(const char *delim = "\r\n");

  /// @brief consume all bytes in readable
  /// @return
  std::vector<char> ReadAll();

  /// @brief read a long integer from buffer and forward the pointer
  /// @param step
  /// @return
  long ReadLongAndForward(size_t &step);

  /// @brief read a long integer but do not forward a pointer
  /// @param step
  /// @return
  long ReadLong(size_t &step);

  /// @brief read a long starting from index
  /// @param index
  /// @param step
  /// @return
  long ReadLongFrom(size_t index, size_t &step);

  /// @brief stream buffer to output file
  /// @param file
  void ToFile(const std::string &file);

 private:
  /// @brief make sure there is enough room
  /// @param n
  void EnsureBytesForWrite(size_t n);

  /// @brief move readable bytes to the head of buffer
  void MoveReadableToHead();

 private:
  std::vector<char> data_;
  size_t p_reader_;
  size_t p_writer_;
};

}  // namespace ahrimq

#endif  // _BUFFER_H_