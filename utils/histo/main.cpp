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

#include <webgl-loader/base.hpp>
#include <webgl-loader/stream.hpp>

using namespace webgl_loader;

void PrintByteHistogram ( FILE* fp ) {
    const size_t kBufSize = 8192;
    char buf[kBufSize];

    BufferedInputStream input ( fp, buf, kBufSize );
    NullSink null_sink;
    ByteHistogramSink histo_sink ( &null_sink );

    while ( kNoError == input.Refill() ) {
        const size_t count = input.end() - input.cursor;
        histo_sink.put_n ( input.cursor, count );
        input.cursor += count;
    }

    const size_t* histo = histo_sink.histo();
    size_t max = 0;
    for ( size_t i = 0; i < 256; ++i ) {
        if ( histo[i] > max ) max = histo[i];
    }

    const char lollipop[] =
        "---------------------------------------------------------------------o";
    const double scale = sizeof ( lollipop ) / double ( max );

    for ( int i = 0; i < 256; ++i ) {
        const int width = scale * histo[i];
        printf ( "%3d: %s\n", i, lollipop + ( sizeof ( lollipop ) - width ) );
    }
}

int main ( int argc, const char* argv[] ) {
    if ( argc != 2 ) {
        fprintf ( stderr, "Usage: %s [in]\n\n"
                  "\tReads binary file |in| and prints byte historgram to STDOUT."
                  "\n\n",
                  argv[0] );
        return -1;
    }
    FILE* fp = fopen ( argv[1], "rb" );
    if ( !fp ) {
        fprintf ( stderr, "Could not open file: %s\n\n", argv[1] );
        return -1;
    }
    PrintByteHistogram ( fp );
    fclose ( fp );
    return 0;
}
