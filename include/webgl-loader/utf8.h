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

#ifndef WEBGL_LOADER_UTF8_H
#define WEBGL_LOADER_UTF8_H

#include <webgl-loader/base.h>
#include <webgl-loader/stream.h>

namespace webgl_loader {

const uint8_t K_UTF8_MORE_BYTES_PREFIX = 0x80;
const uint8_t K_UTF8_TWO_BYTE_PREFIX = 0xC0;
const uint8_t K_UTF8_THREE_BYTE_PREFIX = 0xE0;

const uint16_t K_UTF8_TWO_BYTE_LIMIT = 0x0800;
const uint16_t K_UTF8_SURROGATE_PAIR_START = 0xD800;
const uint16_t K_UTF8_SURROGATE_PAIR_NUM = 0x0800;
const uint16_t K_UTF8_ENCODABLE_END = 0x10000 - K_UTF8_SURROGATE_PAIR_NUM;

const uint16_t K_UTF8_MORE_BYTES_MASK = 0x3F;

bool Uint16ToUtf8 ( uint16_t word, ByteSinkInterface* sink ) {
    if ( word < 0x80 ) {
        sink->Put ( static_cast<char> ( word ) );
    } else if ( word < K_UTF8_TWO_BYTE_LIMIT ) {
        sink->Put ( static_cast<char> ( K_UTF8_TWO_BYTE_PREFIX + ( word >> 6 ) ) );
        sink->Put ( static_cast<char> ( K_UTF8_MORE_BYTES_PREFIX +
                                        ( word & K_UTF8_MORE_BYTES_MASK ) ) );
    } else if ( word < K_UTF8_ENCODABLE_END ) {
        // We can only encode 65535 - 2048 values because of illegal UTF-8
        // characters, such as surrogate pairs in [0xD800, 0xDFFF].
        if ( word >= K_UTF8_SURROGATE_PAIR_START ) {
            // Shift the result to avoid the surrogate pair range.
            word += K_UTF8_SURROGATE_PAIR_NUM;
        }
        sink->Put ( static_cast<char> ( K_UTF8_THREE_BYTE_PREFIX + ( word >> 12 ) ) );
        sink->Put ( static_cast<char> ( K_UTF8_MORE_BYTES_PREFIX +
                                        ( ( word >> 6 ) & K_UTF8_MORE_BYTES_MASK ) ) );
        sink->Put ( static_cast<char> ( K_UTF8_MORE_BYTES_PREFIX +
                                        ( word & K_UTF8_MORE_BYTES_MASK ) ) );
    } else {
        return false;
    }
    return true;
}

}  // namespace webgl_loader

#endif  // WEBGL_LOADER_UTF8_H
