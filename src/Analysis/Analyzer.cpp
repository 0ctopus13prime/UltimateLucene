#include <stdexcept>
#include <Analysis/Analyzer.h>

using namespace lucene::core::analysis;
using namespace lucene::core::analysis::tokenattributes;
using namespace lucene::core::util;
using namespace lucene::core::util::etc;

/**
 *  TokenStreamComponents
 */
TokenStreamComponents::TokenStreamComponents(Tokenizer* source, TokenStream* result)
  : source(source),
    sink(result) {
}

TokenStreamComponents::TokenStreamComponents(Tokenizer* source)
  : source(source),
    sink(source) {
}

void TokenStreamComponents::SetReader(delete_unique_ptr<Reader>& reader) {
  source->SetReader(reader);
}

TokenStream* TokenStreamComponents::GetTokenStream() {
  return sink.get();
}

Tokenizer* TokenStreamComponents::GetTokenizer() {
  return source.get();
}

StringReader& TokenStreamComponents::GetReusableStringReader() {
  return reusable_string_reader;
}

/**
 *  ReuseStrategy
 */
ReuseStrategy::ReuseStrategy() {
}

ReuseStrategy::~ReuseStrategy() {
}

/**
 *  PreDefinedReuseStrategy
 */
PreDefinedReuseStrategy::PreDefinedReuseStrategy() {
}

PreDefinedReuseStrategy::~PreDefinedReuseStrategy() {
}

TokenStreamComponents* PreDefinedReuseStrategy::GetReusableComponents(Analyzer& analyzer, const std::string& field_name) {
  return GetStoredValue<TokenStreamComponents*>(analyzer);
}

void PreDefinedReuseStrategy::SetReusableComponents(Analyzer& analyzer,
                                                    const std::string& field_name,
                                                    TokenStreamComponents* component) {
  // TODO Implement it. std::unique_ptr<TokenStreamComponents>(component)?
}

PreDefinedReuseStrategy PER_FIELD_REUSE_STRATEGY = PreDefinedReuseStrategy();

/**
 *  StringTokenStream
 */
StringTokenStream::StringTokenStream(AttributeFactory* input, std::string& value, size_t length)
  : TokenStream(input),
    value(value),
    length(length),
    used(false),
    term_attribute(AddAttribute<tokenattributes::CharTermAttribute>()),
    offset_attribute(AddAttribute<tokenattributes::OffsetAttribute>()) {
}

void StringTokenStream::Reset() {
  used = false;
}

bool StringTokenStream::IncrementToken() {
  if(used) {
    return false;
  }

  ClearAttributes();
  term_attribute->Append(value);
  offset_attribute->SetOffset(0, length);
  used = true;
  return true;
}

void StringTokenStream::End() {
  TokenStream::End();
  offset_attribute->SetOffset(length, length);
}

/**
 *  Analyzer
 */
Analyzer::Analyzer()
  : Analyzer(static_cast<ReuseStrategy&>(PER_FIELD_REUSE_STRATEGY)) {
}

Analyzer::Analyzer(ReuseStrategy& reuse_strategy)
  : reuse_strategy(reuse_strategy),
    version(Version::LATEST) {
}

Analyzer::~Analyzer() {
}

delete_unique_ptr<Reader> Analyzer::InitReader(const std::string& field_name, delete_unique_ptr<Reader> reader) {
  return reader;
}

delete_unique_ptr<Reader> Analyzer::InitReaderForNormalization(const std::string& field_name, delete_unique_ptr<Reader> reader) {
  return reader;
}

AttributeFactory* Analyzer::GetAttributeFactory(const std::string& field_name) {
  return TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY;
}

delete_unique_ptr<TokenStream> Analyzer::Normalize(const std::string& field_name, delete_unique_ptr<TokenStream> in) {
  return in;
}

TokenStream* Analyzer::GetTokenStream(const std::string& field_name, Reader* reader) {
  TokenStreamComponents* components = reuse_strategy.GetReusableComponents(*this, field_name);
  delete_unique_ptr<Reader> r = InitReader(field_name, std::unique_ptr<Reader, std::function<void(Reader*)>>(reader, std::default_delete<Reader>()));

  if(components == nullptr) {
    components = CreateComponents(field_name);
    reuse_strategy.SetReusableComponents(*this, field_name, components);
  }

  components->SetReader(r);
  return components->GetTokenStream();
}

TokenStream* Analyzer::GetTokenStream(const std::string& field_name, const std::string& text) {
  TokenStreamComponents* components = reuse_strategy.GetReusableComponents(*this, field_name);
  if(components == nullptr) {
    components = CreateComponents(field_name);
    reuse_strategy.SetReusableComponents(*this, field_name, components);
  }

  delete_unique_ptr<Reader> r =
    InitReader(
      field_name,
      delete_unique_ptr<Reader>(
        &components->GetReusableStringReader(),
        [](Reader*){} // No destrucion here, as reusable_string_reader will be reused next time
      )
    );
  components->SetReader(r);
  return components->GetTokenStream();
}

BytesRef Analyzer::Normalize(const std::string& field_name, const std::string& text) {
  StringReader reader(text);
  delete_unique_ptr<Reader> filter_reader = InitReaderForNormalization(
    field_name,
    delete_unique_ptr<Reader>(
      &reader,
      [](Reader*){}
    )
  );

  std::string filtered_text;
  char buffer[256];
  while(true) {
    const int32_t read = filter_reader->Read(buffer, 0, sizeof(buffer));
    if(read == -1) {
      break;
    } else {
      filtered_text.append(buffer, read);
    }
  }

  AttributeFactory* attribute_factory = GetAttributeFactory(field_name);
  StringTokenStream sts(attribute_factory, filtered_text, filtered_text.size());
  delete_unique_ptr<TokenStream> ts = Normalize(field_name,
    delete_unique_ptr<TokenStream>(
      &sts,
      [](TokenStream*){} // No destruction. It's in heap space
    )
  );

  std::shared_ptr<TermToBytesRefAttribute> term_att = ts->AddAttribute<TermToBytesRefAttribute>();
  ts->Reset();

  if(ts->IncrementToken() == false) {
    throw std::runtime_error("The normalization token stream is expected to produce exactly 1 token, but go 0 for analyzer this and input '" + text + "'");
  }

  BytesRef& ref = term_att->GetBytesRef();

  if(ts->IncrementToken()) {
    throw std::runtime_error("The normalization token stream is expected to produce exactly 1 token, but go 0 for analyzer this and input '" + text + "'");
  }

  ts->End();

  return ref;
}

/**
 * StopwordAnalyzerBase
 */
StopwordAnalyzerBase::StopwordAnalyzerBase()
  : Analyzer(),
    stop_words() {
}

StopwordAnalyzerBase::StopwordAnalyzerBase(const characterutil::CharSet& stop_words)
  : Analyzer(),
    stop_words(stop_words) {
}

StopwordAnalyzerBase::StopwordAnalyzerBase(characterutil::CharSet&& stop_words)
  : Analyzer(),
    stop_words(std::forward<characterutil::CharSet>(stop_words)) {
}

StopwordAnalyzerBase::~StopwordAnalyzerBase() {
}

characterutil::CharSet StopwordAnalyzerBase::LoadStopWordSet(const std::string& path) {
  // TODO Implement it
  return characterutil::CharSet();
}

characterutil::CharSet StopwordAnalyzerBase::LoadStopWordSet(const Reader& reader) {
  // TODO Implement it
  return characterutil::CharSet();
}

/**
 * WordlistLoader
 */
WordlistLoader::WordlistLoader() {
}

WordlistLoader::~WordlistLoader() {
}

void WordlistLoader::GetWordSet(Reader& reader, characterutil::CharSet& result) {
  std::string word;
  while(!reader.Eof()) {
    reader.ReadLine(word);
    characterutil::Trim(word);
    result.Add(word);
  }
}

characterutil::CharSet WordlistLoader::GetWordSet(Reader& reader) {
  characterutil::CharSet char_set(INITIAL_CAPACITY, false);
  GetWordSet(reader, char_set);
  return char_set;
}

characterutil::CharSet WordlistLoader::GetWordSet(Reader& reader, std::string& comment) {
  characterutil::CharSet char_set(INITIAL_CAPACITY, false);
  GetWordSet(reader, comment, char_set);
  return char_set;
}

void WordlistLoader::GetWordSet(Reader& reader, std::string& comment, characterutil::CharSet& result) {
  std::string word;
  while(!reader.Eof()) {
    reader.ReadLine(word);
    if(characterutil::IsPrefix(word, comment) == false) {
      characterutil::Trim(word);
      result.Add(word);
    }
  }
}

void GetSnowballWordSet(Reader& reader, characterutil::CharSet& result) {
  std::string line;
  while(!reader.Eof()) {
    reader.ReadLine(line);
    size_t pos = line.find("|");
    if(pos != std::string::npos) {
      line = std::string(line, 0, pos);
    }

    std::vector<std::string> words = characterutil::SplitRegex(line, "\\s+");
    for(const std::string& word : words) {
      if(word.size() > 0) {
        result.Add(word);
      }
    }
  }
}

characterutil::CharSet GetSnowballWordSet(Reader& reader) {
  characterutil::CharSet char_set(16 /*INITIAL_CAPACITY*/, false);
  GetSnowballWordSet(reader, char_set);
  return char_set;
}

characterutil::CharMap<std::string> GetStemDict(Reader& reader, characterutil::CharMap<std::string>& result) {
  std::string line;
  std::string tab_delimiter(1, '\t');
  while(!reader.Eof()) {
    reader.ReadLine(line);
    std::vector<std::string> wordstem = characterutil::Split(line, tab_delimiter, 2);
    result.Put(wordstem[0], wordstem[1]);
  }
}

std::vector<std::string> GetLines(Reader& reader) {
  // TODO Implement it.
  // The reason that leave it as a unimplement function is about lacking of support unicode right now.
  // In Java version, unicode character comparison is required but currently unicode is not supported.
  return {};
}
