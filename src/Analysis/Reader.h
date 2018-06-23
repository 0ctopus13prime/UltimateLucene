#ifndef LUCENE_CORE_ANALYSIS_READER_H_
#define LUCENE_CORE_ANALYSIS_READER_H_

#include <string>
#include <sstream>
#include <exception>

namespace lucene { namespace core { namespace analysis {

class Reader {
  public:
    virtual ~Reader();
    virtual int Read() = 0;
    virtual void ReadLine(std::string& line) = 0;
    virtual int Read(char* cstr, const uint32_t off, const uint32_t len) = 0;
    virtual void Skip(const uint64_t n) = 0;
    virtual bool MarkSupported() = 0;
    virtual void Mark(const uint32_t read_ahead_limit) = 0;
    virtual void Reset() = 0;
    virtual void Close() = 0;
    virtual bool Eof() = 0;
};

class IllegalStateReader: public Reader {
  public:
    virtual ~IllegalStateReader() {}
    int Read() {
      throw std::runtime_error("Thie Read() is not supported in IllegalStateReader");
    }

    void ReadLine(std::string& line) {
      throw std::runtime_error("Thie ReadLine(std::string&) is not supported in IllegalStateReader");
    }

    int32_t Read(char* cstr, const uint32_t off, const uint32_t len) {
      throw std::runtime_error("Thie Read(char*, const uint32_t, const uint32_t) is not supported in IllegalStateReader");
    }

    void Skip(const uint64_t n) {
      throw std::runtime_error("Thie Skip(const uint64_t) is not supported in IllegalStateReader");
    }

    bool MarkSupported() {
      throw std::runtime_error("Thie MarkSupported() is not supported in IllegalStateReader");
    }

    void Mark(uint32_t read_ahead_limit) {
      throw std::runtime_error("Thie Mark(uint32_t) is not supported in IllegalStateReader");
    }

    void Reset() {
      throw std::runtime_error("Thie Reset() is not supported in IllegalStateReader");
    }

    void Close() {
      throw std::runtime_error("Thie Close() is not supported in IllegalStateReader");
    }

    bool Eof() {
      throw std::runtime_error("Thie Eof() is not supported in IllegalStateReader");
    }
};

class StringReader: public Reader {
  private:
    std::istringstream iss;
    uint32_t mark;

  public:
    StringReader();
    StringReader(const StringReader& other);
    StringReader(StringReader&& other);
    StringReader(const std::string& str);
    StringReader(const char* cstr);
    StringReader(const char* cstr, const unsigned len);
    StringReader(const char* cstr, const uint32_t off, const unsigned len);
    StringReader(std::istringstream& iss);
    StringReader(std::istringstream&& iss);
    StringReader& operator=(StringReader& other);
    StringReader& operator=(StringReader&& other);
    StringReader& operator=(std::istringstream& new_iss);
    StringReader& operator=(std::istringstream&& new_iss);
    virtual ~StringReader();
    void SetValue(const std::string& value);
    void SetValue(const char* cstr);
    void SetValue(const char* cstr, const uint32_t len);
    void SetValue(const char* cstr, const uint32_t off, const uint32_t len);
    int Read() override;
    void ReadLine(std::string& line) override;
    int Read(char* cstr, const uint32_t off, const uint32_t len) override;
    void Skip(const size_t n) override;
    bool MarkSupported() override;
    void Mark(const uint32_t read_ahead_limit) override;
    void Reset() override;
    void Close() override;
    bool Eof() override;
};

}}}

#endif
