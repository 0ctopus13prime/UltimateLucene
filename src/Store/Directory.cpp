#include <Store/Directory.h>

using lucene::core::store::Directory;
using lucene::core::store::IOUtils;
                                         
void
IOUtils::DeleteFilesIgnoringExceptions(Directory& dir,
                                       const std::vector<std::string>& files) {
  for (const std::string& file_name : files) {
    try {
      dir.DeleteFile(file_name);
    } catch(...) {
      // Nothing
    }
  }
}
