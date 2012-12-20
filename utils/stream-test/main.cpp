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
#include <webgl-loader/stream.hpp>

#include <iostream>  // DO NOT SUBMIT
#include <string>

using namespace webgl_loader;

class BufferedInputTest {
public:
    void TestFromMemory() {
        const char bytes[] = { 1, 2, 3, 4, 5 };
        BufferedInput bi ( bytes, sizeof ( bytes ) );
        assert ( K_NO_ERROR == bi.error() );
        int sum = 0;
        while ( bi.cursor != bi.end() ) {
            sum += *bi.cursor++ ;
        }
        assert ( 15 == sum );
        assert ( K_NO_ERROR == bi.error() );
        bi.refill();
        assert ( bi.begin() == bi.cursor );
        assert ( K_END_OF_FILE == bi.error() );
        sum = 0;
        while ( bi.cursor != bi.end() ) {
            sum += *bi.cursor++;
        }
        assert ( 0 == sum );
    }
};

int main() {
    BufferedInputTest tester;
    tester.TestFromMemory();

    return 0;
}
