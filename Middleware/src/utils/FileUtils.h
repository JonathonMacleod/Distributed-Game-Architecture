#ifndef MIDDLEWARE__FILEUTILS__H
    #define MIDDLEWARE__FILEUTILS__H

    #include <string>

    #include "Logging.h"

    namespace Middleware::Utils {
        
        std::string ReadFileContents(const std::string& filepath);

    }

#endif