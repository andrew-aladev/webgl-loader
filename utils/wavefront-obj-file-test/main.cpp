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

#include <cstring>

#define private public

#include <webgl-loader/base.hpp>
#include <webgl-loader/mesh.hpp>

#define CHECK_INDICES(POSITION_INDEX, TEXCOORD_INDEX, NORMAL_INDEX) \
  assert(POSITION_INDEX == position_index_);                         \
  assert(TEXCOORD_INDEX == texcoord_index_);                         \
  assert(NORMAL_INDEX == normal_index_)

#define PARSE_INDICES(LINE) \
  assert(LINE + strlen(LINE) == ParseIndices(LINE))

using namespace webgl_loader;

class ParseIndicesTester {
public:
    ParseIndicesTester()
        : position_index_ ( 0 ),
          texcoord_index_ ( 0 ),
          normal_index_ ( 0 ) {
    }

    void Test() {
        assert ( NULL == ParseIndices ( "" ) );
        assert ( NULL == ParseIndices ( "nodigit" ) );
        const char kBasic[] = "1/2/3";
        PARSE_INDICES ( kBasic );
        CHECK_INDICES ( 1, 2, 3 );
        const char kNoTexCoord[] = "4//5";
        PARSE_INDICES ( kNoTexCoord );
        CHECK_INDICES ( 4, 0, 5 );
        const char kUno[] = "6";
        PARSE_INDICES ( kUno );
        CHECK_INDICES ( 6, 0, 0 );
        const char kBad[] = "7/?/8";
        ParseIndices ( kBad );
        CHECK_INDICES ( 7, 0, 0 );
        const char kThree[] = " 1/122/1 2/123/2 3/117/3";
        const char* next = ParseIndices ( kThree );
        CHECK_INDICES ( 1, 122, 1 );
        next = ParseIndices ( next );
        CHECK_INDICES ( 2, 123, 2 );
        assert ( kThree + strlen ( kThree ) == ParseIndices ( next ) );
        CHECK_INDICES ( 3, 117, 3 );
    }

private:
    const char* ParseIndices ( const char* line ) {
        return obj_.ParseIndices ( line, 0, &position_index_, &texcoord_index_,
                                   &normal_index_ );
    }

    int position_index_;
    int texcoord_index_;
    int normal_index_;
    WavefrontObjFile obj_;
};

int main ( int argc, char* argv[] ) {
    ParseIndicesTester tester;
    tester.Test();
    return 0;
}
