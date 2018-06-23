#include <type_traits>
#include <cctype>
#include <stdexcept>
#include <Analysis/AttributeImpl.h>
#include <Analysis/CharacterUtil.h>
#include <Analysis/TokenStream.h>
#include <Util/ArrayUtil.h>

using namespace lucene::core::analysis;
using namespace lucene::core::analysis::tokenattributes;
using namespace lucene::core::util;
using namespace lucene::core::util::arrayutil;

/**
 *  TokenStream
 */

AttributeFactory* TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY =
  AttributeFactory::GetStaticImplementation<AttributeFactory::DefaultAttributeFactory, tokenattributes::PackedTokenAttributeImpl>();

TokenStream::TokenStream()
  : AttributeSource() {
}

TokenStream::TokenStream(const AttributeSource& input)
  : AttributeSource() {
  input.ShallowCopyTo(*this);
}

TokenStream::TokenStream(AttributeFactory& factory)
  : AttributeSource(factory) {
}

TokenStream::~TokenStream() {
}

void TokenStream::End() {
  EndAttributes();
}

void TokenStream::Close() {
}

/**
 *  TokenFilter
 */

TokenFilter::TokenFilter(std::unique_ptr<TokenStream>&& input)
  : TokenStream(*input),
    input(std::forward<std::unique_ptr<TokenStream>>(input)) {
}

TokenFilter::TokenFilter(TokenStream* input)
  : TokenStream(*input),
    input(input) {
}

TokenFilter::~TokenFilter() {
}

void TokenFilter::End() {
  input->End();
}

void TokenFilter::Reset() {
  input->Reset();
}

/**
 *  Tokenizer
 */
IllegalStateReader Tokenizer::ILLEGAL_STATE_READER;

Tokenizer::Tokenizer()
  : TokenStream(),
    input(ILLEGAL_STATE_READER),
    input_pending(ILLEGAL_STATE_READER) {
}

Tokenizer::Tokenizer(AttributeFactory& factory)
  : TokenStream(factory),
    input(ILLEGAL_STATE_READER),
    input_pending(ILLEGAL_STATE_READER) {
}

Tokenizer::~Tokenizer() {
}

uint32_t Tokenizer::CorrectOffset(const uint32_t current_off) {
  if(characterutil::CharFilter* char_filter = dynamic_cast<characterutil::CharFilter*>(&input)) {
    char_filter->CorrectOffset(current_off);
  } else {
    return current_off;
  }
}

void Tokenizer::SetReader(Reader& input) {
  if(dynamic_cast<IllegalStateReader*>(&input) == nullptr) {
    throw std::runtime_error("TokenStream contract violation: close() call missing");
  }

  input_pending = input;
  SetReaderTestPoint();
}

void Tokenizer::Reset() {
  input = std::move(input_pending);
}

void Tokenizer::Close() {
  input.Close();
  input = ILLEGAL_STATE_READER;
  input_pending = ILLEGAL_STATE_READER;
}

void Tokenizer::SetReaderTestPoint() {
}

/**
 * LowerCaseFilter
 */

LowerCaseFilter::LowerCaseFilter(TokenStream* in)
  : TokenFilter(in),
    term_att(AddAttribute<tokenattributes::CharTermAttribute>()) {
}

LowerCaseFilter::~LowerCaseFilter() {
}

bool LowerCaseFilter::IncrementToken() {
  if(input->IncrementToken()) {
    characterutil::ToLowerCase(term_att->Buffer(), 0, term_att->Length());
    return true;
  } else {
    return false;
  }
}

/**
 * CachingTokenFilter
 */
CachingTokenFilter::CachingTokenFilter(TokenStream* in)
  : TokenFilter(in),
    cache(64),
    iterator(cache.begin()),
    final_state(),
    first_time(true) {
}

CachingTokenFilter::~CachingTokenFilter() {
}

void CachingTokenFilter::End() {
  if(AttributeSource::State* final_state_ptr = final_state.get()) {
    RestoreState(final_state_ptr);
  }
}

void CachingTokenFilter::Reset() {
  if(first_time) {
    input->Reset();
  } else {
    iterator = cache.begin();
  }
}

bool CachingTokenFilter::IncrementToken() {
  if(first_time) {
    first_time = false;
    FillCache();
  }

  if(iterator == cache.end()) {
    return false;
  }

  RestoreState((*iterator).get());
  iterator++;
  return true;
}

void CachingTokenFilter::FillCache() {
  while(input->IncrementToken()) {
    cache.push_back(std::unique_ptr<AttributeSource::State>(CaptureState()));
  }

  input->End();
  final_state = std::unique_ptr<AttributeSource::State>(CaptureState());
}

bool CachingTokenFilter::IsCached() {
  return !first_time;
}

/**
 * FilteringTokenFilter
 */
FilteringTokenFilter::FilteringTokenFilter(TokenStream* in)
  : TokenFilter(in),
    pos_incr_attr(AddAttribute<PositionIncrementAttribute>()),
    skipped_positions(0) {
}

FilteringTokenFilter::~FilteringTokenFilter() {
}

bool FilteringTokenFilter::IncrementToken() {
  skipped_positions = 0;
  while(input->IncrementToken()) {
    if(Accept()) {
      if(skipped_positions != 0) {
        pos_incr_attr->SetPositionIncrement(pos_incr_attr->GetPositionIncrement() + skipped_positions);
      }
      return true;
    }

    skipped_positions += pos_incr_attr->GetPositionIncrement();
  }

  return false;
}

void FilteringTokenFilter::Reset() {
  TokenFilter::Reset();
  skipped_positions = 0;
}

void FilteringTokenFilter::End() {
  TokenFilter::End();
  pos_incr_attr->SetPositionIncrement(pos_incr_attr->GetPositionIncrement() + skipped_positions);
}


/**
 *  StopFilter
 */
StopFilter::StopFilter(TokenStream* in, characterutil::CharSet& stop_words)
  : FilteringTokenFilter(in),
    stop_words(stop_words),
    term_att(AddAttribute<tokenattributes::CharTermAttribute>()) {
}

StopFilter::StopFilter(TokenStream* in, characterutil::CharSet&& stop_words)
  : FilteringTokenFilter(in),
    stop_words(std::forward<characterutil::CharSet>(stop_words)),
    term_att(AddAttribute<tokenattributes::CharTermAttribute>()) {
}

StopFilter::~StopFilter() {
}

bool StopFilter::Accept() {
  return stop_words.Contains(term_att->Buffer(), 0, term_att->Length());
}
