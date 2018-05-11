#include <Analysis/CharacterUtil.h>
#include <Analysis/TokenStream.h>
#include <ctype.h>

using namespace lucene::core::analysis;
using namespace lucene::core::util;

/**
 *  TokenStream
 */

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
