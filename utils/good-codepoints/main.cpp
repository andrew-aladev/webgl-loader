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
//   Google Inc (wonchun@gmail.com)
//   andrew.aladjev@gmail.com

#include <cstdio>

#include <webgl-loader/stream.hpp>
#include <webgl-loader/utf8.hpp>

using namespace webgl_loader;

int main ( int argc, char* argv[] ) {
    if ( argc != 2 ) {
        fprintf ( stderr, "Usage: %s out.utf8\n\n"
                  "Generates all legal UTF-8 codepoints from 0 to 65,536, excluding\n"
                  "the surrogate pair range [0xD800, 0xDFFF].\n",
                  argv[0] );
        return -1;
    }
    FILE* fp = fopen ( argv[1], "wb" );
    FileSink sink ( fp );
    for ( size_t word = 0; word < 65536; ++word ) {
        uint16_to_utf8 ( word, &sink );
    }
    fclose ( fp );
    return 0;
}
