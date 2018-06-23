#include <cstring>
#include <algorithm>
#include <iostream>
#include <Analysis/Reader.h>
#include <Analysis/CharacterUtil.h>

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

StringReader::StringReader(const char* cstr)
  : StringReader(cstr, 0, std::strlen(cstr)) {
}

StringReader::StringReader(const char* cstr, const unsigned len)
  : StringReader(cstr, 0, len) {
}

StringReader::StringReader(const char* cstr, const uint32_t off, const unsigned len)
  : iss(std::string(cstr, off, len)),
    mark(0) {
}

StringReader::StringReader(std::istringstream& iss)
  : iss(iss.str()),
    mark(0) {
}

StringReader::StringReader(std::istringstream&& iss)
  : iss(std::forward<std::istringstream>(iss)),
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

void StringReader::SetValue(const std::string& value) {
  iss.clear(); // If eofbit is set, clear this state
  iss.str(value);
}

void StringReader::SetValue(const char* cstr) {
  SetValue(std::string(cstr));
}

void StringReader::SetValue(const char* cstr, const uint32_t len) {
  SetValue(cstr, 0, len);
}

void StringReader::SetValue(const char* cstr, const uint32_t off, const uint32_t len) {
  std::string value(cstr + off, len);
  SetValue(value);
}

int StringReader::Read() {
  return iss.get();
}

void StringReader::ReadLine(std::string& line) {
  std::getline(iss, line);
}

int32_t StringReader::Read(char* buf, const uint32_t off, const uint32_t len) {
  if(iss.eof()) {
    return -1;
  }

  uint32_t read = 0;
  for(uint32_t i = off; i < len ; ++i) {
    int got = iss.get();
    if(got != -1) {
      buf[i] = got;
      ++read;
    } else {
      break;
    }
  }

  return read;
}

void StringReader::Skip(const uint64_t n) {
  iss.ignore(n);
}

bool StringReader::MarkSupported() {
  return true;
}

void StringReader::Mark(const uint32_t read_ahead_limit) {
  mark = read_ahead_limit;
}

void StringReader::Reset() {
  const int32_t pos = iss.tellg();
  iss.seekg(0, iss.end);
  const int32_t length = iss.tellg();

  if(mark < length) {
    iss.clear();
    iss.seekg(mark);
  } else {
    iss.seekg(0, iss.end);
    iss.setstate(std::ios::eofbit);
  }
}

void StringReader::Close() {
  iss.setstate(std::ios::eofbit);
}

bool StringReader::Eof() {
  return iss.eof();
}
