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

#include <webgl-loader/base.hpp>

using namespace webgl_loader;

int main() {
    char buf[9] = { '\0' };
    for ( size_t i = 0; i < 8; ++i ) {
        for ( size_t j = 1; j < 16; ++j ) {
            const uint32_t w = j << ( 4*i );
            ToHex ( w, buf );
            puts ( buf );
        }
    }
    return 0;
}
