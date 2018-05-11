#include <type_traits>
#include <ctype.h>
#include <stdexcept>
#include <Analysis/AttributeImpl.h>
#include <Analysis/CharacterUtil.h>
#include <Analysis/TokenStream.h>

using namespace lucene::core::analysis;
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

unsigned int Tokenizer::CorrectOffset(const unsigned int current_off) {
  Reader* reader = input.get();
  if(dynamic_cast<characterutil::CharFilter*>(reader)) {
    characterutil::CharFilter* char_filter = dynamic_cast<characterutil::CharFilter*>(reader);
    // char_filter->CorrectOffset(current_off);
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
