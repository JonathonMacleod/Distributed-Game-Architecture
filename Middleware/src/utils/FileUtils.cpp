#include "FileUtils.h"

#include <fstream>
#include <sstream>
#include <filesystem>

//
// FileUtils.h implementations
//

namespace Middleware {

    std::string Utils::ReadFileContents(const std::string& filepath) {
        MIDDLEWARE_TRACE("About to attempt to read from file using path '%s'. Current working directory is '%s'", filepath.c_str(), std::filesystem::current_path().string().c_str());

        std::ifstream fileStream(filepath);
        if (!fileStream.is_open()) {
            MIDDLEWARE_ERROR("Failed to open file at filepath '%s'", filepath);
            return "";
        }
        MIDDLEWARE_TRACE("Have successfully managed to open file '%s'", filepath.c_str());

        std::string line;
        std::stringstream assembledFileContents;
        bool requireNewline = false;
        while (std::getline(fileStream, line)) {
            if (requireNewline) {
                assembledFileContents << "\n";
            }

            assembledFileContents << line;
            requireNewline = true;
        }

        fileStream.close();
        MIDDLEWARE_TRACE("Have finished reading file contents. Closing file...");

        return assembledFileContents.str();
    }

}
