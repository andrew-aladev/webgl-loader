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

#ifndef WEBGL_LOADER_UTF8_H_
#define WEBGL_LOADER_UTF8_H_

#include <webgl-loader/base.h>
#include <webgl-loader/stream.h>

namespace webgl_loader {

const uint8_t kUtf8MoreBytesPrefix = 0x80;
const uint8_t kUtf8TwoBytePrefix = 0xC0;
const uint8_t kUtf8ThreeBytePrefix = 0xE0;

const uint16_t kUtf8TwoByteLimit = 0x0800;
const uint16_t kUtf8SurrogatePairStart = 0xD800;
const uint16_t kUtf8SurrogatePairNum = 0x0800;
const uint16_t kUtf8EncodableEnd = 0x10000 - kUtf8SurrogatePairNum;

const uint16_t kUtf8MoreBytesMask = 0x3F;

bool Uint16ToUtf8 ( uint16_t word, ByteSinkInterface* sink ) {
    if ( word < 0x80 ) {
        sink->Put ( static_cast<char> ( word ) );
    } else if ( word < kUtf8TwoByteLimit ) {
        sink->Put ( static_cast<char> ( kUtf8TwoBytePrefix + ( word >> 6 ) ) );
        sink->Put ( static_cast<char> ( kUtf8MoreBytesPrefix +
                                        ( word & kUtf8MoreBytesMask ) ) );
    } else if ( word < kUtf8EncodableEnd ) {
        // We can only encode 65535 - 2048 values because of illegal UTF-8
        // characters, such as surrogate pairs in [0xD800, 0xDFFF].
        if ( word >= kUtf8SurrogatePairStart ) {
            // Shift the result to avoid the surrogate pair range.
            word += kUtf8SurrogatePairNum;
        }
        sink->Put ( static_cast<char> ( kUtf8ThreeBytePrefix + ( word >> 12 ) ) );
        sink->Put ( static_cast<char> ( kUtf8MoreBytesPrefix +
                                        ( ( word >> 6 ) & kUtf8MoreBytesMask ) ) );
        sink->Put ( static_cast<char> ( kUtf8MoreBytesPrefix +
                                        ( word & kUtf8MoreBytesMask ) ) );
    } else {
        return false;
    }
    return true;
}

}  // namespace webgl_loader

#endif  // WEBGL_LOADER_UTF8_H_
