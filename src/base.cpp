// This file is part of WebGL Loader.
//
// WebGL Loader is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// WebGL Loader is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with WebGL Loader.
// If not, see <http://www.gnu.org/licenses/>.
//
// Copyright
//   Google Inc (wonchun@gmail.com)
//   andrew.aladjev@gmail.com

#include <webgl-loader/base.hpp>

namespace webgl_loader {

// Jenkin's One-at-a-time Hash. Not the best, but simple and
// portable.
uint32_t SimpleHash ( char *key, size_t len, uint32_t seed) {
    uint32_t hash = seed;
    for ( size_t i = 0; i < len; ++i ) {
        hash += static_cast<unsigned char> ( key[i] );
        hash += ( hash << 10 );
        hash ^= ( hash >> 6 );
    }
    hash += ( hash << 3 );
    hash ^= ( hash >> 11 );
    hash += ( hash << 15 );
    return hash;
}

void ToHex ( uint32_t w, char out[9] ) {
    const char k_offset0  = '0';
    const char k_offset10 = 'a' - 10;
    out[8] = '\0';
    for ( size_t i = 8; i > 0; ) {
        uint32_t bits = w & 0xF;
        out[--i] = bits + ( ( bits < 10 ) ? k_offset0 : k_offset10 );
        w >>= 4;
    }
}

uint16_t Quantize ( float f, float in_min, float in_scale, uint16_t out_max ) {
    return static_cast<uint16_t> ( out_max * ( ( f-in_min ) / in_scale ) );
}

} // namespace webgl_loader
