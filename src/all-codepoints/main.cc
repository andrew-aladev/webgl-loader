// This file is part of WebGL Loader.
//
// WebGL Loader is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// WebGL Loader is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with WebGL Loader.
// If not, see <http://www.gnu.org/licenses/>.
//
// Copyright
//   Google Inc
//   andrew.aladjev@gmail.com

#include <stdio.h>

#include <webgl-loader/utf8.h>

namespace webgl_loader {

// Dumps all codepoints without any concern for the surrogate pair range.
// Some code duplication from Uint16ToUtf8 as a result.
void DumpAllCodepoints ( FILE* fp ) {
    for ( size_t word = 0; word != 0x80; ++word ) {
        PutChar ( word, fp );
    }
    for ( size_t word = 0x80; word != 0x800; ++word ) {
        PutChar ( K_UTF8_TWO_BYTE_PREFIX + static_cast<char> ( word >> 6 ), fp );
        PutChar ( K_UTF8_MORE_BYTES_PREFIX +
                  static_cast<char> ( word & K_UTF8_MORE_BYTES_MASK ), fp );
    }
    for ( size_t word = 0x800; word != 0x10000; ++word ) {
        PutChar ( K_UTF8_THREE_BYTE_PREFIX + static_cast<char> ( word >> 12 ), fp );
        PutChar ( K_UTF8_MORE_BYTES_PREFIX +
                  static_cast<char> ( ( word >> 6 ) & K_UTF8_MORE_BYTES_MASK ), fp );
        PutChar ( K_UTF8_MORE_BYTES_PREFIX +
                  static_cast<char> ( word & K_UTF8_MORE_BYTES_MASK ), fp );
    }
}

}  // namespace webgl_loader

int main ( int argc, char* argv[] ) {
    if ( argc != 2 ) {
        fprintf ( stderr, "Usage: %s out.utf8\n\n"
                  "Generates all UTF-8 codepoints from 0 to 65,536, including\n"
                  "the surrogate pair range [0xD800, 0xDFFF].\n",
                  argv[0] );
        return -1;
    }
    FILE* fp = fopen ( argv[1], "wb" );
    webgl_loader::DumpAllCodepoints ( fp );
    fclose ( fp );
    return 0;
}
