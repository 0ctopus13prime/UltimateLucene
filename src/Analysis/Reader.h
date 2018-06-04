#ifndef LUCENE_CORE_ANALYSIS_READER_H_
#define LUCENE_CORE_ANALYSIS_READER_H_

#include <string>
#include <sstream>

namespace lucene { namespace core { namespace analysis {

class Reader {
  public:
    virtual ~Reader();
    virtual char Read() = 0;
    virtual std::string ReadLine() = 0;
    virtual int32_t Read(char* buf, const uint32_t off, const uint32_t len) = 0;
    virtual size_t Skip(const unsigned long n) = 0;
    virtual bool MarkSupported() = 0;
    virtual void Mark(uint32_t read_ahead_limit) = 0;
    virtual void Reset() = 0;
    virtual void Close() = 0;
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
    StringReader(const char* buf, const unsigned len);
    StringReader(const char* buf, const uint32_t off, const unsigned len);
    StringReader(std::istringstream&& iss);
    StringReader& operator=(StringReader& other);
    StringReader& operator=(StringReader&& other);
    StringReader& operator=(std::istringstream& new_iss);
    StringReader& operator=(std::istringstream&& new_iss);
    virtual ~StringReader();
    void SetValue(std::string& value);
    void SetValue(char* buf, const uint32_t len);
    void SetValue(char* buf, const uint32_t off, const uint32_t len);
    char Read() override;
    std::string ReadLine() override;
    int32_t Read(char* buf, const uint32_t off, const uint32_t len) override;
    unsigned long Skip(const unsigned long n) override;
    bool MarkSupported() override;
    void Mark(uint32_t read_ahead_limit) override;
    void Reset() override;
    void Close() override;
};

}}}

#endif
