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

#ifndef WEBGL_LOADER_JSON_HPP
#define WEBGL_LOADER_JSON_HPP

#include <webgl-loader/stream.hpp>

namespace webgl_loader {

// JsonSink will generate JSON in the ByteSink passed in, but does
// not actually own the backing data. Performs rudimentary grammar
// checking. It will automatically add delimiting punctuation and
// prevent non-String object keys.
//
// TODO: Pretty printing.
class JsonSink {
private:
    // Conservative estimate of how many bytes to allocate on the stack
    // as scratch space for int/float to NUL-terminated string.
    static const int K_BUF_SIZE = 32;

    // JsonSink needs to internally maintain some structural state in
    // order to correctly delimit values with the appropriate
    // punctuation. |State| indicates what the desired next value should
    // be.
    enum State {
        JSON_STATE_OBJECT_VALUE,
        JSON_STATE_OBJECT_KEY_FIRST,
        JSON_STATE_OBJECT_KEY,
        JSON_STATE_ARRAY_FIRST,
        JSON_STATE_ARRAY,
        JSON_STATE_SIMPLE
    };

    ByteSinkInterface* sink_;
    std::vector<State> state_;

public:
    explicit JsonSink ( ByteSinkInterface* sink );
    ~JsonSink();

    void put_null();
    void put_bool ( bool b );
    void put_int ( int i );
    void put_float ( float f );
    void put_string ( const char* str );
    void begin_array();
    void begin_object();
    void end();
    void end_all();

private:
    State get_state() const;
    void set_state ( State state );
    void push_state ( State state );
    void pop_state();
    void update_state();
    void check_not_key() const;
    void on_put_value();
    void put ( char c );
    void put_n ( const char* str, size_t len );
};

}  // namespace webgl_loader

#endif  // WEBGL_LOADER_JSON_HPP
