#include <Analysis/Reader.h>

using namespace lucene::core::analysis;

/**
 *  Reader
 */
Reader::~Reader() {
}

/**
 * StringReader
 */

StringReader::StringReader()
  : iss(),
    mark(0) {
}

StringReader::StringReader(const StringReader& other)
  : iss(other.iss.str()),
    mark(other.mark) {
}

StringReader::StringReader(StringReader&& other)
  : iss(std::move(other.iss)),
    mark(other.mark) {
}

StringReader::StringReader(const std::string& str)
  : iss(str),
    mark(0) {
}

StringReader::StringReader(const char* buf, const unsigned len)
  : StringReader(buf, 0, len) {
}

StringReader::StringReader(const char* buf, const uint32_t off, const unsigned len)
  : iss(std::string(buf, off, len)),
    mark(0) {
}

StringReader& StringReader::operator=(StringReader& other) {
  if(this != &other) {
    iss.clear();
    iss.str(other.iss.str());
    mark = other.mark;
  }
}

StringReader& StringReader::operator=(StringReader&& other) {
  if(this != &other) {
    iss = std::move(other.iss);
    mark = other.mark;
  }
}

StringReader& StringReader::operator=(std::istringstream& new_iss) {
  iss.str(new_iss.str());
  mark = 0;
}

StringReader& StringReader::operator=(std::istringstream&& new_iss) {
  iss = std::move(new_iss);
  mark = 0;
}

StringReader::~StringReader() {
}

void StringReader::SetValue(std::string& value) {
  iss.clear();
  iss.str(value);
}

void StringReader::SetValue(char* buf, const uint32_t len) {
  SetValue(buf, 0, len);
}

void StringReader::SetValue(char* buf, const uint32_t off, const uint32_t len) {
  std::string value(buf + off, len);
  SetValue(value);
}

char StringReader::Read() {
  char ch = -1;
  if(!iss.eof()) {
    iss >> ch;
  }

  return ch;
}

std::string StringReader::ReadLine() {
  std::string ret;
  if(!iss.eof()) {
    iss >> ret;
  }

  return ret;
}

int32_t StringReader::Read(char* buf, const uint32_t off, const uint32_t len) {
  iss.readsome(buf + off, len);
}

size_t StringReader::Skip(const unsigned long n) {
  iss.seekg(n);
}

bool StringReader::MarkSupported() {
  return true;
}

void StringReader::Mark(uint32_t read_ahead_limit) {
  mark = read_ahead_limit;
}

void StringReader::Reset() {
  iss.seekg(mark);
}

void StringReader::Close() {
  iss.seekg(0, iss.end);
}
