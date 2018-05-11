#include <stdexcept>
#include <Analysis/Analyzer.h>

using namespace lucene::core::analysis;
using namespace lucene::core::util;

/**
 *  TokenStreamComponents
 */
TokenStreamComponents::(Tokenizer* source, TokenStream* result)
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

TokenStream& TokenStreamComponents::GetTokenStream() {
  return *sink;
}

Tokenizer& TokenStreamComponents::GetTokenizer() {
  return *source;
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

TokenStreamComponents& PreDefinedReuseStrategy::GetReusableComponents() {
  // TODO Implement it.
}

void PreDefinedReuseStrategy::SetReusableComponents(Analyzer* analyzer,
                                                    const std::string& field_name,
                                                    TokenStreamComponents* component) {
  // TODO Implement it.
}

PER_FIELD_REUSE_STRATEGY = PreDefinedReuseStrategy();

/**
 *  StringTokenStream
 */
StringTokenStream::StringTokenStream(const AttributeSource& input, std::string& value, size_t length)
  : TokenStream(input),
    value(value),
    length(length),
    used(false),
    term_attribute(AddAttribute<CharTermAttribute>()),
    offset_attribute(AddAttribute<OffsetAttribute>()) {
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
  offset_attribute.SetOffset(0, length);
  used = true;
  return true;
}

void StringTokenStream::End() {
  TokenStream::End();
  offset_attribute.SetOffset(length, length);
}

/**
 *  Analyzer
 */
Analyzer::Analyzer()
  : Analyzer(&PER_FIELD_REUSE_STRATEGY) {
}

Analyzer::Analyzer(ReuseStrategy* reuse_strategy)
  : reuse_strategy(reuse_strategy),
    version(Version.LATEST) {
}

delete_unique_ptr<Reader> Analyzer::InitReader(const std::string field_name, delete_unique_ptr<Reader> reader) {
  return reader;
}

delete_unique_ptr<Reader> Analyzer::InitReaderForNormalization(const std::string field_name, delete_unique_ptr<Reader> reader) {
  return reader;
}

TokenStream Analyzer::Normalize(const std::string& field_name, TokenStream& in) {
  return in;
}

TokenStream* Analyzer::TokenStream(const std::string& field_name, Reader* reader) {
  TokenStreamComponents* components = reuse_strategy->GetReusableComponents();
  std::unique_ptr<Reader> r = InitReader(field_name, std::unique_ptr<Reader, std::function<void(Reader*)>>(reader, std::default_delete<Reader>()));

  if(components == nullptr) {
    components = CreateComponents(field_name);
    reuse_strategy->SetReusableComponents(this, field_name, components);
  }

  components->SetReader(r);
  return components->GetTokenStream();
}

TokenStream* Analyzer::TokenStream(const std::string field_name, const std::string& text) {
  TokenStreamComponents* components = reuse_strategy->GetReusableComponents();
  if(components == nullptr) {
    components = CreateComponents(field_name);
    reuse_strategy->SetReusableComponents(this, field_name, components);
  }

  std::unique_ptr<Reader> r =
    InitReader(field_name,
      delete_unique_ptr<Reader>(
        &components->reusable_string_reader,
        [](Reader*){} // No destrucion here, as reusable_string_reader will be reused next time
      )
    );
  components->SetReader(r);
  return components->GetTokenStream();
}

void Analyzer::Normalize(const std::string& field_name, const std::string& text, BytesRef& target) {
  StringReader reader(text);
  delete_unique_ptr<Reader> filter_reader = InitReaderForNormalization(
    field_name,
    delete_unique_ptr<Reader>(
      &reader,
      [](Reader*){}
    )
  );

  std::filtered_text;
  char buffer[256];
  while(true) {
    const int read = filter_reader.read(buffer, 0, sizeof(buffer));
    if(read == -1) {
      break;
    } else {
      filtered_text.append(buffer, read);
    }
  }

  AttributeSource& attribute_factory = AttributeFactory(field_name);
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
