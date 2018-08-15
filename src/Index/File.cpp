#include <Index/File.h>

using lucene::core::index::IndexFileNames;

/**
 *  IndexFileNames
 */

std::string IndexFileNames::FileNameFromGeneration(const std::string& base,
                                                   const std::string& ext,
                                                   const uint64_t gen) {
  // TODO(0ctopus13prime): Implement this
}

std::string IndexFileNames::SegmentFileName(const std::string& segment_name,
                                            const std::string& segment_suffix,
                                            const std::string& ext) {
  if (!ext.empty() || !segment_name.empty()) {
    std::string ret;
    ret.reserve(segment_name.length() + 2 +
                segment_suffix.length() +
                ext.length());

    ret += segment_name;
    if (!segment_suffix.empty()) {
      ret += '_';    
      ret += segment_suffix;
    }

    if (!ext.empty()) {
      ret += '.';
      ret += ext;
    }

    return ret;
  } else {
    return segment_name;
  }
}

bool IndexFileNames::MatchesExtension(const std::string& file_name,
                                      const std::string& ext) {
  // TODO(0ctopus13prime): Implement this
  return true;
}

uint32_t IndexFileNames::IndexOfSegmentName(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return 0;
}

std::string IndexFileNames::StripSegmentName(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return "";
}

uint64_t IndexFileNames::ParseGeneration(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return 0L;
}

std::string IndexFileNames::ParseSegmentName(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return "";
}

std::string IndexFileNames::StripExtension(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return "";
}

std::string IndexFileNames::GetExtension(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return "";
}
