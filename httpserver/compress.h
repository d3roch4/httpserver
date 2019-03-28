#ifndef COMPRESS_H
#define COMPRESS_H
#include <sstream>

#include <string>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <zlib.h>
using std::string;
using std::stringstream;

// Found these here http://mail-archives.apache.org/mod_mbox/trafficserver-dev/201110.mbox/%3CCACJPjhYf=+br1W39vyazP=ix
//eQZ-4Gh9-U6TtiEdReG3S4ZZng@mail.gmail.com%3E
#define MOD_GZIP_ZLIB_WINDOWSIZE 15
#define MOD_GZIP_ZLIB_CFACTOR    9
#define MOD_GZIP_ZLIB_BSIZE      8096

// Found this one here: http://panthema.net/2007/0328-ZLibString.html, author is Timo Bingmann
// edited version
/** Compress a STL string using zlib with given compression level and return
  * the binary data. */
std::string compress_gzip(const std::string& str,
                             int compressionlevel = Z_BEST_COMPRESSION);

// Found this one here: http://panthema.net/2007/0328-ZLibString.html, author is Timo Bingmann
/** Compress a STL string using zlib with given compression level and return
  * the binary data. */
std::string compress_deflate(const std::string& str,
                            int compressionlevel = Z_BEST_COMPRESSION);

/** Decompress an STL string using zlib and return the original data. */
std::string decompress_deflate(const std::string& str);

std::string decompress_gzip(const std::string& str);


// Compile: g++ -std=c++11 % -lz

#endif // COMPRESS_H
