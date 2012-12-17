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

#include <webgl-loader/base.h>
#include <webgl-loader/stream.h>

#include <iostream>  // DO NOT SUBMIT
#include <string>

namespace webgl_loader {

class BufferedInputTest {
public:
    void TestFromMemory() {
        const char bytes[] = { 1, 2, 3, 4, 5 };
        BufferedInput bi ( bytes, sizeof ( bytes ) );
        CHECK ( kNoError == bi.error() );
        int sum = 0;
        while ( bi.cursor != bi.end() ) {
            sum += *bi.cursor++ ;
        }
        CHECK ( 15 == sum );
        CHECK ( kNoError == bi.error() );
        bi.Refill();
        CHECK ( bi.begin() == bi.cursor );
        CHECK ( kEndOfFile == bi.error() );
        sum = 0;
        while ( bi.cursor != bi.end() ) {
            sum += *bi.cursor++;
        }
        CHECK ( 0 == sum );
    }
};

}  // namespace webgl_loader

int main() {
    webgl_loader::BufferedInputTest tester;
    tester.TestFromMemory();

    return 0;
}
