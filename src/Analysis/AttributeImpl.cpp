#include <Analysis/AttributeImpl.h>

using namespace lucene::core::analysis::tokenattributes;

/**
 * BytesTermAttributeImpl
 */

BytesTermAttributeImpl::BytesTermAttributeImpl()
  : bytes() {
}

BytesTermAttributeImpl::BytesTermAttributeImpl(const BytesTermAttributeImpl& other)
  : bytes(other.bytes) {
}

BytesRef& BytesTermAttributeImpl::GetBytesRef() {
  return bytes;
}

void BytesTermAttributeImpl::SetBytesRef(BytesRef& bytes) {
  
}

void BytesTermAttributeImpl::Clear() {

}

void BytesTermAttributeImpl::CopyTo(AttributeImpl& target) {

}

void BytesTermAttributeImpl::ReflectWith(AttributeReflector& reflector) {

}

bool BytesTermAttributeImpl::operator==(BytesTermAttributeImpl& other) {
  return true;
}
