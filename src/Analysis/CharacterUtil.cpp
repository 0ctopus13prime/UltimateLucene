#include <cstring>
#include <cctype>
#include <Analysis/CharacterUtil.h>

using namespace lucene::core::analysis::characterutil;

void lucene::core::analysis::characterutil::ToLowerCase(char* buffer, const uint32_t offset, const uint32_t limit) {
  for(uint32_t i = offset ; i < offset + limit ; ++i) {
    buffer[i] = tolower(buffer[i]);
  }
}

/**
 * CharFilter
 */
CharFilter::CharFilter(Reader* input)
  : input(input) {
}

CharFilter::~CharFilter() {
  Close();
}

void CharFilter::Close() {
  input->Close();
}

int32_t CharFilter::CorrectOffset(int32_t current_off) {
  const int32_t corrected = Correct(current_off);

  if(CharFilter* char_filter = dynamic_cast<CharFilter*>(input.get())) {
    return char_filter->CorrectOffset(corrected);
  } else {
    return corrected;
  }
}

/**
 *  CharPtrRangeInfo
 */
CharPtrRangeInfo::CharPtrRangeInfo(const char* str, const uint32_t offset, const uint32_t length, const bool is_tmp)
  : is_tmp(is_tmp),
    str(is_tmp ? str : lucene::core::util::arrayutil::CopyOfRange(str, offset, length)),
    offset(offset),
    length(length) {
}

CharPtrRangeInfo::CharPtrRangeInfo(const CharPtrRangeInfo& other)
  : is_tmp(other.is_tmp),
    str(other.is_tmp ? other.str : lucene::core::util::arrayutil::CopyOfRange(other.str, other.offset, other.length)),
    offset(other.offset),
    length(other.length) {
}

CharPtrRangeInfo::CharPtrRangeInfo(CharPtrRangeInfo&& other)
  : is_tmp(other.is_tmp),
    str(other.str),
    offset(other.offset),
    length(other.length) {
  // Prevent deallocation of str from other
}

CharPtrRangeInfo::~CharPtrRangeInfo() {
  if(!is_tmp) {
    delete[] str;
  }
}

/**
 *  CharPtrRangeInfoEqual
 */
CharPtrRangeInfoEqual::CharPtrRangeInfoEqual(const bool ignore_case/*= false*/)
  : ignore_case(ignore_case) {
}

bool CharPtrRangeInfoEqual::operator()(const CharPtrRangeInfo& o1, const CharPtrRangeInfo& o2) const {
  if(o1.length != o2.length) {
    return false;
  }

  const char* str_o1 = o1.str + o1.offset;
  const char* str_o2 = o2.str + o2.offset;

  if(ignore_case) {
    for(uint32_t i = 0 ; i < o1.length ; ++i) {
      if(std::tolower(str_o1[i]) != std::tolower(str_o2[i])) {
        return false;
      }
    }

    return true;
  } else {
    return (std::memcmp(str_o1, str_o2, o1.length) == 0);
  }
}

/**
 *  CharPtrRangeInfoHasher
 */
CharPtrRangeInfoHasher::CharPtrRangeInfoHasher(const bool ignore_case/*= false*/)
  : ignore_case(ignore_case) {
}

CharPtrRangeInfoHasher::CharPtrRangeInfoHasher(const CharPtrRangeInfoHasher& other)
  : ignore_case(other.ignore_case) {
}

size_t CharPtrRangeInfoHasher::operator()(const CharPtrRangeInfo& char_ptr_range_info) const {
  int32_t code = 0;
  const int32_t stop = char_ptr_range_info.offset + char_ptr_range_info.length;

  if(ignore_case) {
    for(int32_t i = char_ptr_range_info.offset ; i < stop ; ) {
      code = code * 31 + std::tolower(char_ptr_range_info.str[i]);
      i++;
    }
  } else {
    for(int32_t i = char_ptr_range_info.offset ; i < stop ; ++i) {
      code = code * 31 + char_ptr_range_info.str[i];
    }
  }

  return code;
}

/**
 *  CharSet
 */
CharSet::CharSet(const bool ignore_case/*= false*/)
  : ignore_case(ignore_case),
    internal_set(32, CharPtrRangeInfoHasher(ignore_case), CharPtrRangeInfoEqual(ignore_case)) {
}

CharSet::CharSet(std::vector<std::string>& c, const bool ignore_case)
  : ignore_case(ignore_case),
    internal_set(c.size(), CharPtrRangeInfoHasher(ignore_case), CharPtrRangeInfoEqual(ignore_case)) {
  for(const std::string& str : c) {
    Add(str);
  }
}

CharSet::CharSet(const uint32_t start_size, const bool ignore_case)
  : ignore_case(ignore_case),
    internal_set(start_size, CharPtrRangeInfoHasher(ignore_case), CharPtrRangeInfoEqual(ignore_case)) {
}

CharSet::CharSet(const CharSet& other)
  : ignore_case(other.ignore_case),
    internal_set(other.internal_set) {
}

CharSet::CharSet(const CharSet&& other)
  : ignore_case(other.ignore_case),
    internal_set(std::move(other.internal_set)) {
}

CharSet::~CharSet() {
}

CharSet& CharSet::operator=(const CharSet& other) {
  ignore_case = other.ignore_case;
  internal_set = other.internal_set;
}

CharSet& CharSet::operator=(const CharSet&& other) {
  ignore_case = other.ignore_case;
  internal_set = std::move(other.internal_set);
}

void CharSet::Clear() {
  internal_set.clear();
}

bool CharSet::IgnoreCase() {
  return ignore_case;
}

bool CharSet::Contains(const std::string& str) {
  CharPtrRangeInfo info(str.c_str(), 0, str.size(), true);
  auto it = internal_set.find(info);
  return (it != internal_set.end());
}

bool CharSet::Contains(const char* str, uint32_t offset, uint32_t length) {
  CharPtrRangeInfo info(str, offset, length, true);
  auto it = internal_set.find(info);
  return (it != internal_set.end());
}

bool CharSet::Add(const char* str, uint32_t offset, uint32_t length) {
  CharPtrRangeInfo info(lucene::core::util::arrayutil::CopyOfRange(str, offset, length), offset, length);
  auto pair = internal_set.insert(std::move(info));
  return pair.second;
}

bool CharSet::Add(const std::string& str) {
  CharPtrRangeInfo info(lucene::core::util::arrayutil::CopyOfRange(str.c_str(), 0, str.size()), 0, str.size());
  auto pair = internal_set.insert(std::move(info));
  return pair.second;
}

size_t CharSet::Size() {
  return internal_set.size();
}