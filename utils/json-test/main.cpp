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
#include <webgl-loader/json.hpp>

#include <float.h>
#include <limits.h>
#include <string>

using namespace webgl_loader;

class JsonSinkTest {
public:
    JsonSinkTest()
        : sink_ ( &buf_ ), json_ ( &sink_ ) {
    }

    void TestNull() {
        json_.put_null();
        CheckString ( "null" );
    }

    void TestBool() {
        json_.put_bool ( true );
        CheckString ( "true" );
        json_.put_bool ( false );
        CheckString ( "false" );
    }

    void TestInt() {
        for ( int i = 0; i < 10; ++i ) {
            json_.put_int ( i );
            char test[] = "0";
            test[0] += i;
            CheckString ( test );
        }
        json_.put_int ( INT_MIN );
        CheckString ( "-2147483648" );
        json_.put_int ( INT_MAX );
        CheckString ( "2147483647" );
    }

    void TestFloat() {
        json_.put_float ( 123.456 );
        CheckString ( "123.456" );
        json_.put_float ( FLT_MAX );
        CheckString ( "3.40282e+38" );
        json_.put_float ( -FLT_MAX );
        CheckString ( "-3.40282e+38" );
        json_.put_float ( FLT_MIN );
        CheckString ( "1.17549e-38" );
        json_.put_float ( -FLT_MIN );
        CheckString ( "-1.17549e-38" );
    }

    void TestString() {
        json_.put_string ( "foo" );
        CheckString ( "\"foo\"" );
    }

    void TestArray() {
        json_.begin_array();
        for ( int i = 0; i < 100; i += 10 ) {
            json_.put_int ( i );
        }
        json_.end();
        CheckString ( "[0,10,20,30,40,50,60,70,80,90]" );

        json_.begin_array();
        json_.begin_array();
        json_.put_null();
        json_.end();
        json_.begin_array();
        json_.put_bool ( false );
        json_.put_bool ( true );
        json_.end();
        for ( int i = 0; i < 5; ++i ) {
            json_.put_int ( i*i );
        }
        json_.begin_object();
        json_.put_string ( "key" );
        json_.put_string ( "value" );
        json_.end_all();

        CheckString ( "[[null],[false,true],0,1,4,9,16,{\"key\":\"value\"}]" );
    }

    void TestObject() {
        json_.begin_object();
        json_.put_string ( "key1" );
        json_.put_int ( 1 );
        json_.put_string ( "keyABC" );
        json_.put_string ( "abc" );
        json_.end();

        CheckString ( "{\"key1\":1,\"keyABC\":\"abc\"}" );

        json_.begin_object();
        json_.put_string ( "array" );
        json_.begin_array();
        for ( int i = 1; i <= 3; ++i ) {
            json_.put_int ( i );
        }
        json_.end();
        json_.begin_object();
        json_.put_string ( "key" );
        json_.put_string ( "value" );
        json_.end();
        json_.put_string ( "k" );
        json_.put_float ( 0.1 );
        json_.end();

        CheckString ( "{\"array\":[1,2,3]{\"key\":\"value\"},\"k\":0.1}" );
    }

private:
    void CheckString ( const char* str ) {
        assert ( buf_ == str );
        buf_.clear();
    }

    std::string buf_;
    StringSink sink_;
    JsonSink json_;
};

int main() {
    JsonSinkTest tester;
    tester.TestNull();
    tester.TestBool();
    tester.TestInt();
    tester.TestFloat();
    tester.TestString();
    tester.TestArray();
    tester.TestObject();

    return 0;
}
