#ifndef LUCENE_CORE_ANALYSIS_CHARACTERUTILS
#define LUCENE_CORE_ANALYSIS_CHARACTERUTILS

#include <string>
#include <vector>
#include <memory>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <Analysis/Reader.h>
#include <Util/ArrayUtil.h>

namespace lucene { namespace core { namespace analysis { namespace characterutil {

void ToLowerCase(char* buffer, const uint32_t offset, const uint32_t limit);

class CharFilter: public Reader {
  private:
    std::unique_ptr<Reader> input;

  public:
    CharFilter(Reader* input);
    virtual ~CharFilter();
    void Close();
    virtual int32_t Correct(int32_t current_off) = 0;
    int32_t CorrectOffset(int32_t current_off);
};

class CharPtrRangeInfo {
  public:
    const bool is_tmp;
    const char* str;
    const uint32_t offset;
    const uint32_t length;

    CharPtrRangeInfo(const char* str, const uint32_t offset, const uint32_t length, const bool is_tmp = false);
    CharPtrRangeInfo(const CharPtrRangeInfo& other);
    CharPtrRangeInfo(CharPtrRangeInfo&& other);
    ~CharPtrRangeInfo();
};

class CharPtrRangeInfoEqual {
  private:
    bool ignore_case;

  public:
    CharPtrRangeInfoEqual(const bool ignore_case = false);
    bool operator()(const CharPtrRangeInfo& o1, const CharPtrRangeInfo& o2) const;
};

class CharPtrRangeInfoHasher {
  private:
    bool ignore_case;

  public:
    CharPtrRangeInfoHasher(const bool ignore_case = false);
    CharPtrRangeInfoHasher(const CharPtrRangeInfoHasher& other);
    size_t operator()(const CharPtrRangeInfo& char_ptr_range_info) const;
};

template <typename VALUE>
class CharMap {
  private:
    bool ignore_case;

    using INTERNAL_MAP = std::unordered_map<CharPtrRangeInfo, VALUE, CharPtrRangeInfoHasher, CharPtrRangeInfoEqual>;

    INTERNAL_MAP internal_map;

  public:
    CharMap(const bool ignore_case=false)
      : CharMap(32, ignore_case) {
    }

    CharMap(const uint32_t start_size, const bool ignore_case=false)
      : ignore_case(ignore_case),
        internal_map(start_size, CharPtrRangeInfoHasher(ignore_case), CharPtrRangeInfoEqual(ignore_case)) {
    }

    template<typename InputIt>
    CharMap(InputIt start, InputIt end, const bool ignore_case=false)
      : ignore_case(ignore_case),
        internal_map(start, end, std::distance(start, end), CharPtrRangeInfoHasher(ignore_case), CharPtrRangeInfoEqual(ignore_case)) {
    }

    CharMap(const CharMap& other)
      : ignore_case(other.ignore_case),
        internal_map(other.internal_map) {
    }

    CharMap(const CharMap&& other)
      : ignore_case(other.ignore_case),
        internal_map(std::move(other.internal_map)) {
    }

    ~CharMap() {
    }

    CharMap& operator=(const CharMap& other) {
      ignore_case = other.ignore_case;
      internal_map = other.ingernal_map;
    }

    CharMap& operator=(const CharMap&& other) {
      ignore_case = other.ignore_case;
      internal_map = std::move(other.internal_map);
    }

    void Clear() {
      internal_map.clear();
    }

    bool IgnoreCase() {
      return ignore_case;
    }

    bool ContainsKey(const std::string& str) {
      CharPtrRangeInfo info(str.c_str(), 0, str.size(), true);
      return (internal_map.find(info) != internal_map.end());
    }

    bool ContainsKey(const char* str, uint32_t offset, uint32_t length) {
      CharPtrRangeInfo info(str, offset, length, true);
      return (internal_map.find(info) != internal_map.end());
    }

    typename INTERNAL_MAP::iterator Get(const char* str, uint32_t offset, uint32_t length) {
      CharPtrRangeInfo info(str, offset, length, true);
      return internal_map.find(info);
    }

    typename INTERNAL_MAP::iterator Get(const std::string& str) {
      CharPtrRangeInfo info(str.c_str(), 0, str.size(), true);
      return internal_map.find(info);
    }

    bool Put(const char* str, uint32_t offset, uint32_t length, VALUE& value) {
      CharPtrRangeInfo info(lucene::core::util::arrayutil::CopyOfRange(str, offset, length), offset, length);
      auto it = internal_map.insert(std::make_pair(std::move(info), value));
      return it->second;
    }

    bool Put(const char* str, uint32_t offset, uint32_t length, VALUE&& value) {
      CharPtrRangeInfo info(lucene::core::util::arrayutil::CopyOfRange(str, offset, length), offset, length);
      auto it = internal_map.insert(std::make_pair(std::move(info), std::forward(value)));
      return it->second;
    }

    bool Put(const std::string& str, VALUE& value) {
      CharPtrRangeInfo info(lucene::core::util::arrayutil::CopyOfRange(str.c_str(), 0, str.size()), 0, str.size());
      auto it = internal_map.insert(std::make_pair(std::move(info), value));
      return it->second;
    }

    bool Put(const std::string& str, VALUE&& value) {
      CharPtrRangeInfo info(lucene::core::util::arrayutil::CopyOfRange(str.c_str(), 0, str.size()), 0, str.size());
      auto it = internal_map.insert(std::make_pair(std::move(info), std::forward(value)));
      return it->second;
    }

    uint32_t Size() {
      return internal_map.size();
    }

    typename INTERNAL_MAP::iterator Iterator() {
      return internal_map.begin();
    }

    typename INTERNAL_MAP::iterator End() {
      return internal_map.end();
    }
};

class CharSet {
  private:
    bool ignore_case;
    using INTERNAL_SET = std::unordered_set<CharPtrRangeInfo, CharPtrRangeInfoHasher, CharPtrRangeInfoEqual>;
    INTERNAL_SET internal_set;

  public:
    CharSet(const bool ignore_case = false);
    CharSet(std::vector<std::string>& c, const bool ignore_case);
    CharSet(const uint32_t start_size, const bool ignore_case);
    CharSet(const CharSet& other);
    CharSet(const CharSet&& other);
    ~CharSet();
    CharSet& operator=(const CharSet& other);
    CharSet& operator=(const CharSet&& other);
    void Clear();
    bool IgnoreCase();
    bool Contains(const std::string& str);
    bool Contains(const char* str, uint32_t offset, uint32_t length);
    bool Add(const char* str, uint32_t offset, uint32_t length);
    bool Add(const std::string& str);
    size_t Size();
    typename INTERNAL_SET::iterator Iterator();
    typename INTERNAL_SET::iterator End();
};

void Trim(std::string& str);

}}}}

#endif
