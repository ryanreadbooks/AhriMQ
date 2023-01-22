#include "buffer.h"
#include <algorithm>
#include <fstream>

namespace ahrimq {

void Buffer::Append(const std::string &value) {
  Append(value.data(), value.size());
}

void Buffer::Append(const char *value, int len) {
  EnsureBytesForWrite(len);
  // append value to writable
  memcpy(BeginWrite(), value, len);
  // update p_writer
  p_writer_ += len;
}

void Buffer::Reset() {
  memset(data_.data(), 0, data_.capacity());
  p_reader_ = 0;
  p_writer_ = 0;
}

void Buffer::EnsureBytesForWrite(size_t n) {
  size_t w = WritableBytes();
  if (WritableBytes() < n) {
    size_t cur_free_space = PrependableBytes() + WritableBytes();
    if (cur_free_space >= n) {
      MoveReadableToHead();
    } else {
      // need to alloc more space
      std::vector<char> new_place(p_writer_ + n + 2, 0);
      // log original readable size to update p_writer_ after memcpy
      size_t r = ReadableBytes();
      // discard prependable
      memcpy(new_place.data(), BeginRead(), r);
      data_.swap(new_place);  // swap new and old space to save memory
      p_reader_ = 0;
      p_writer_ = r;
    }
  }
}

void Buffer::MoveReadableToHead() {
  memcpy(data_.data(), data_.data() + p_reader_, ReadableBytes());
  size_t can_free = PrependableBytes();
  memset(data_.data() + p_writer_ - can_free, 0, can_free);
  // update pointer
  p_reader_ = 0;
  p_writer_ = p_writer_ - can_free;
}

void Buffer::ReaderIdxForward(size_t len) {
  len = std::min(len, ReadableBytes());
  p_reader_ += len;
}

void Buffer::ReaderIdxBackward(size_t len) {
  len = std::min(len, PrependableBytes());
  p_reader_ -= len;
}

int Buffer::FindCRLFInReadable() {
  auto it_start = data_.begin() + p_reader_;
  auto it_end = data_.begin() + p_writer_;
  auto it = std::search(it_start, it_end, CRLF, CRLF + 2);
  return (it == it_end) ? -1 : (it - it_start);  // offset with respect to p_reader_
}

std::string Buffer::ReadStringAndForward(size_t len) {
  if (len > ReadableBytes()) {
    return "";
  }
  std::string ans(BeginRead(), len);
  ReaderIdxForward(len);
  return ans;
}

std::string Buffer::ReadStringAndForwardTill(const char *delim) {
  auto it_begin = data_.begin() + p_reader_;
  auto it_end = data_.begin() + p_writer_;
  size_t delim_len = strlen(delim);
  auto it = std::search(it_begin, it_end, delim, delim + delim_len);
  if (it == it_end) {
    return "";
  }
  const char *begin_read = BeginRead();
  size_t offset = it - it_begin;
  ReaderIdxForward(offset + delim_len);
  return std::string(begin_read, offset);
}

std::vector<char> Buffer::ReadAll() {
  size_t n = ReadableBytes();
  if (n == 0) {
    return {};
  }
  std::vector<char> copy;
  copy.reserve(n);
  copy.insert(copy.begin(), BeginRead(), BeginRead() + n);
  ReaderIdxForward(n);
  return std::move(copy);
}

std::string Buffer::ReadString(size_t len) {
  if (len > ReadableBytes()) {
    return "";
  }
  return std::string(BeginRead(), len);
}

std::string Buffer::ReadStringFrom(size_t index, size_t len) {
  if (index >= ReadableBytes()) {
    return "";
  }
  len = std::min(len, ReadableBytes() - index);
  return std::string(BeginRead() + index, len);
}

char Buffer::ReadableCharacterAt(size_t index) const {
  // return the char at index in readable
  index = std::min(index, ReadableBytes() - 1);  // prevent overflow
  return data_[BeginReadIdx() + index];
}

long Buffer::ReadLongAndForward(size_t &step) {
  // convert readable bytes to int till not digit
  char *p_end;
  long num = strtoll(BeginRead(), &p_end, 10);
  step = p_end - BeginRead();
  ReaderIdxForward(step);
  return num;
}

long Buffer::ReadLong(size_t &step) {
  char *p_end;
  long num = strtoll(BeginRead(), &p_end, 10);
  // if ok == 0, then no long number can be read from here
  step = p_end - BeginRead();
  return num;
}

long Buffer::ReadLongFrom(size_t index, size_t &step) {
  char *p_end;
  if (index >= ReadableBytes()) {
    step = 0;
    return 0;
  }
  long num = strtoll(BeginRead() + index, &p_end, 10);
  step = p_end - (BeginRead() + index);
  return num;
}

void Buffer::ToFile(const std::string &file) {
  // flush readable bytes to file
  std::ofstream fs(file, std::ios::app);
  fs.write(BeginRead(), ReadableBytes());
}

}  // namespace ahrimq