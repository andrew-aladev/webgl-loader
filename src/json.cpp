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

#include <webgl-loader/json.hpp>

namespace webgl_loader {

// |sink| is unowned and should not be NULL.
JsonSink::JsonSink ( ByteSinkInterface* sink )
    : sink_ ( sink ) {
    state_.reserve ( 8 );
    this->push_state ( JSON_STATE_SIMPLE );
}

// Automatically close values when JsonSink goes out of scope.
JsonSink::~JsonSink() {
    this->end_all();
}

// Methods to put scalar values into the JSON object. Aside from
// PutString, these should not be called when JsonSink expects an
// object key.
void JsonSink::put_null() {
    this->on_put_value();
    this->put_n ( "null", 4 );
}

void JsonSink::put_bool ( bool b ) {
    this->on_put_value();
    this->put_n ( b ? "true" : "false", b ? 4 : 5 );
}

void JsonSink::put_int ( int i ) {
    this->on_put_value();
    char buf[K_BUF_SIZE];
    int32_t len = sprintf ( buf, "%d", i );
    assert ( len > 0 && len < K_BUF_SIZE );
    this->put_n ( buf, len );
}

void JsonSink::put_float ( float f ) {
    this->on_put_value();
    char buf[K_BUF_SIZE];
    int32_t len = sprintf ( buf, "%g", f );
    assert ( len > 0 && len < K_BUF_SIZE );
    this->put_n ( buf, len );
}

// |str| should not be NULL.
void JsonSink::put_string ( const char* str ) {
    // Strings are the only legal value for object keys.
    switch ( this->get_state() ) {
    case JSON_STATE_OBJECT_KEY:
        this->put ( ',' ); // fall through.
    case JSON_STATE_OBJECT_KEY_FIRST:
        this->set_state ( JSON_STATE_OBJECT_VALUE );
        break;
    default:
        this->update_state();
    }

    // TODO: escaping.
    this->put ( '\"' );
    this->put_n ( str, strlen ( str ) );
    this->put ( '\"' );
}

// Arrays and Objects are recursive JSON values.
void JsonSink::begin_array() {
    this->on_put_value();
    this->put ( '[' );
    this->push_state ( JSON_STATE_ARRAY_FIRST );
}

void JsonSink::begin_object() {
    this->on_put_value();
    this->put ( '{' );
    this->push_state ( JSON_STATE_OBJECT_KEY_FIRST );
}

// Close recurisve value, if necessary. Will automatically pad null
// values to unmatched object keys, if necessary.
void JsonSink::end() {
    switch ( this->get_state() ) {
    case JSON_STATE_OBJECT_VALUE:
        // We haven't provided a value, so emit a null..
        this->put_null();  // ...and fall through to the normal case.
    case JSON_STATE_OBJECT_KEY:
    case JSON_STATE_OBJECT_KEY_FIRST:
        this->put ( '}' );
        break;
    case JSON_STATE_ARRAY_FIRST:
    case JSON_STATE_ARRAY:
        this->put ( ']' );
        break;
    default:
        return;  // Do nothing.
    }
    this->pop_state();
}

// Close all values. Convenient way to end up with a valid JSON object.
void JsonSink::end_all() {
    while ( this->get_state() != JSON_STATE_SIMPLE ) {
        this->end();
    }
}

// State stack helpers. Because JSON allows for recurisve values, maintain a stack of State enums.
JsonSink::State JsonSink::get_state() const {
    return this->state_.back();
}

void JsonSink::set_state ( State state ) {
    this->state_.back() = state;
}

void JsonSink::push_state ( State state ) {
    this->state_.push_back ( state );
}

void JsonSink::pop_state() {
    this->state_.pop_back();
}

// State transducer.
void JsonSink::update_state() {
    switch ( this->get_state() ) {
    case JSON_STATE_OBJECT_VALUE:
        this->put ( ':' );
        this->set_state ( JSON_STATE_OBJECT_KEY );
        return;
    case JSON_STATE_ARRAY_FIRST:
        this->set_state ( JSON_STATE_ARRAY );
        return;
    case JSON_STATE_ARRAY:
        this->put ( ',' );
        return;
    default:
        return;
    }
}

void JsonSink::check_not_key() const {
    assert ( this->get_state() != JSON_STATE_OBJECT_KEY_FIRST ||
             this->get_state() != JSON_STATE_OBJECT_KEY );
}

// A helper method for scalars (besides strings) that prevents them from being used as object keys.
void JsonSink::on_put_value() {
    this->check_not_key();
    this->update_state();
}

// Convenience forwarding methods for sink_.
void JsonSink::put ( char c ) {
    this->sink_->put ( c );
}

void JsonSink::put_n ( const char* str, size_t len ) {
    this->sink_->put_n ( str, len );
}

} // namespace webgl_loader
