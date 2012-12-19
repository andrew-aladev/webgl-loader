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

#ifndef WEBGL_LOADER_UTF8_HPP
#define WEBGL_LOADER_UTF8_HPP

#include <webgl-loader/base.hpp>
#include <webgl-loader/stream.hpp>

namespace webgl_loader {

const uint8_t K_UTF8_MORE_BYTES_PREFIX = 0x80;
const uint8_t K_UTF8_TWO_BYTE_PREFIX   = 0xC0;
const uint8_t K_UTF8_THREE_BYTE_PREFIX = 0xE0;

const uint16_t K_UTF8_TWO_BYTE_LIMIT       = 0x0800;
const uint16_t K_UTF8_SURROGATE_PAIR_START = 0xD800;
const uint16_t K_UTF8_SURROGATE_PAIR_NUM   = 0x0800;
const uint16_t K_UTF8_ENCODABLE_END        = 0x10000 - K_UTF8_SURROGATE_PAIR_NUM;

const uint16_t K_UTF8_MORE_BYTES_MASK      = 0x3F;

bool uint16_to_utf8 ( uint16_t word, ByteSinkInterface* sink );

}  // namespace webgl_loader

#endif  // WEBGL_LOADER_UTF8_HPP
