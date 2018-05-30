#include <type_traits>
#include <ctype.h>
#include <stdexcept>
#include <Analysis/AttributeImpl.h>
#include <Analysis/CharacterUtil.h>
#include <Analysis/TokenStream.h>

using namespace lucene::core::analysis;
using namespace lucene::core::analysis::tokenattributes;
using namespace lucene::core::util;

/**
 *  TokenStream
 */

AttributeFactory* TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY =
  AttributeFactory::GetStaticImplementation<std::remove_pointer<decltype(AttributeFactory::DEFAULT_ATTRIBUTE_FACTORY)>::type, tokenattributes::PackedTokenAttributeImpl>();

TokenStream::TokenStream()
  : AttributeSource() {
}

TokenStream::TokenStream(const AttributeSource& input)
  : AttributeSource(input) {
}

TokenStream::TokenStream(AttributeFactory* factory)
  : AttributeSource(factory) {
}

TokenStream::~TokenStream() {
}

void TokenStream::End() {
  EndAttributes();
}

/**
 *  TokenFilter
 */

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
Tokenizer::Tokenizer()
  : TokenStream() {
}

Tokenizer::Tokenizer(AttributeFactory* factory)
  : TokenStream(factory) {
}

Tokenizer::~Tokenizer() {
}

uint32_t Tokenizer::CorrectOffset(const uint32_t current_off) {
  Reader* reader = input.get();
  if(characterutil::CharFilter* char_filter = dynamic_cast<characterutil::CharFilter*>(reader)) {
    char_filter->CorrectOffset(current_off);
  } else {
    return current_off;
  }
}

void Tokenizer::SetReader(delete_unique_ptr<Reader>& input) {
  if(input.get() != nullptr) {
    throw std::runtime_error("TokenStream contract violation: Close() call missing");
  }

  input_pending = std::move(input);
  SetReaderTestPoint();
}

void Tokenizer::Reset() {
  input.reset(input_pending.release());
}

void Tokenizer::Close() {
  input.reset();
  input_pending.reset();
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
    first_time(true) {
}

CachingTokenFilter::~CachingTokenFilter() {
}

void CachingTokenFilter::End() {
  if(final_state.attribute.use_count() > 0 || final_state.next) {
    RestoreState(final_state);
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

  RestoreState(*iterator++);
  return true;
}

void CachingTokenFilter::FillCache() {
  while(input->IncrementToken()) {
    cache.push_back(CaptureState());
  }

  input->End();
  final_state = CaptureState();
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
