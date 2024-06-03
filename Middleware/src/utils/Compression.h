#ifndef MIDDLEWARE__COMPRESSION__H
    #define MIDDLEWARE__COMPRESSION__H

    #include <iostream>
    #include <vector>
    #include <boost/iostreams/filtering_stream.hpp>
    #include <boost/iostreams/filter/zlib.hpp>
    #include <boost/iostreams/device/back_inserter.hpp>

    #include "Logging.h"

    namespace Middleware::Utils {
        
        inline std::vector<char>* CompressMemory(const char* data, size_t length) {
            // Create a Boost stream filter that uses the ZLib compression library to compress the data passed into it
            boost::iostreams::filtering_ostream streamCompressor;
            streamCompressor.push(boost::iostreams::zlib_compressor());

            // Tell Boost to take any data passed to the compression algorithm and push the result into a char vector
            std::vector<char>* compressedData = new std::vector<char>();
            streamCompressor.push(boost::iostreams::back_inserter(*compressedData));

            // Pass the uncompressed data into the stream filter to be compressed
            streamCompressor.write(data, length);
            streamCompressor.flush();

            return compressedData;
        }

        inline std::vector<char>* DecompressMemory(const char* data, size_t length) {
            // Create a Boost stream filter that uses the ZLib compression library to decompress the data passed into it
            boost::iostreams::filtering_istream streamDecompressor;
            streamDecompressor.push(boost::iostreams::zlib_decompressor());

            // Push the compressed data into the stream filter to be decompressed, with the output being passed into a char vector
            streamDecompressor.push(boost::iostreams::array_source(data, length));
            std::vector<char>* decompressedData = new std::vector<char>(std::istreambuf_iterator<char>(streamDecompressor), {});

            return decompressedData;
        }

    }

#endif