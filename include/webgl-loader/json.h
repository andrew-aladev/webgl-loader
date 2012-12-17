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

#ifndef WEBGL_LOADER_JSON_H
#define WEBGL_LOADER_JSON_H

#include <float.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include <webgl-loader/base.h>
#include <webgl-loader/stream.h>

namespace webgl_loader {

// JsonSink will generate JSON in the ByteSink passed in, but does
// not actually own the backing data. Performs rudimentary grammar
// checking. It will automatically add delimiting punctuation and
// prevent non-String object keys.
//
// TODO: Pretty printing.
class JsonSink {
public:
    // |sink| is unowned and should not be NULL.
    explicit JsonSink ( ByteSinkInterface* sink )
        : sink_ ( sink ) {
        state_.reserve ( 8 );
        PushState ( JSON_STATE_SIMPLE );
    }

    // Automatically close values when JsonSink goes out of scope.
    ~JsonSink() {
        EndAll();
    }

    // Methods to put scalar values into the JSON object. Aside from
    // PutString, these should not be called when JsonSink expects an
    // object key.
    void PutNull() {
        OnPutValue();
        PutN ( "null", 4 );
    }

    void PutBool ( bool b ) {
        OnPutValue();
        PutN ( b ? "true" : "false", b ? 4 : 5 );
    }

    void PutInt ( int i ) {
        OnPutValue();
        char buf[kBufSize];
        int len = sprintf ( buf, "%d", i );
        assert ( len > 0 && len < kBufSize );
        PutN ( buf, len );
    }

    void PutFloat ( float f ) {
        OnPutValue();
        char buf[kBufSize];
        int len = sprintf ( buf, "%g", f );
        assert ( len > 0 && len < kBufSize );
        PutN ( buf, len );
    }

    // |str| should not be NULL.
    void PutString ( const char* str ) {
        // Strings are the only legal value for object keys.
        switch ( GetState() ) {
        case JSON_STATE_OBJECT_KEY:
            Put ( ',' ); // fall through.
        case JSON_STATE_OBJECT_KEY_FIRST:
            SetState ( JSON_STATE_OBJECT_VALUE );
            break;
        default:
            UpdateState();
        }

        // TODO: escaping.
        Put ( '\"' );
        PutN ( str, strlen ( str ) );
        Put ( '\"' );
    }

    // Arrays and Objects are recursive JSON values.
    void BeginArray() {
        OnPutValue();
        Put ( '[' );
        PushState ( JSON_STATE_ARRAY_FIRST );
    }

    void BeginObject() {
        OnPutValue();
        Put ( '{' );
        PushState ( JSON_STATE_OBJECT_KEY_FIRST );
    }

    // Close recurisve value, if necessary. Will automatically pad null
    // values to unmatched object keys, if necessary.
    void End() {
        switch ( GetState() ) {
        case JSON_STATE_OBJECT_VALUE:
            // We haven't provided a value, so emit a null..
            PutNull();  // ...and fall through to the normal case.
        case JSON_STATE_OBJECT_KEY:
        case JSON_STATE_OBJECT_KEY_FIRST:
            Put ( '}' );
            break;
        case JSON_STATE_ARRAY_FIRST:
        case JSON_STATE_ARRAY:
            Put ( ']' );
            break;
        default:
            return;  // Do nothing.
        }
        PopState();
    }

    // Close all values. Convenient way to end up with a valid JSON
    // object.
    void EndAll() {
        while ( GetState() != JSON_STATE_SIMPLE ) End();
    }

private:
    // Conservative estimate of how many bytes to allocate on the stack
    // as scratch space for int/float to NUL-terminated string.
    static const int kBufSize = 32;

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

    // State stack helpers. Because JSON allows for recurisve values,
    // maintain a stack of State enums.
    State GetState() const {
        return state_.back();
    }

    void SetState ( State state ) {
        state_.back() = state;
    }

    void PushState ( State state ) {
        state_.push_back ( state );
    }

    void PopState() {
        state_.pop_back();
    }

    // State transducer.
    void UpdateState() {
        switch ( GetState() ) {
        case JSON_STATE_OBJECT_VALUE:
            Put ( ':' );
            SetState ( JSON_STATE_OBJECT_KEY );
            return;
        case JSON_STATE_ARRAY_FIRST:
            SetState ( JSON_STATE_ARRAY );
            return;
        case JSON_STATE_ARRAY:
            Put ( ',' );
            return;
        default:
            return;
        }
    }

    void CheckNotKey() const {
        assert ( GetState() != JSON_STATE_OBJECT_KEY_FIRST ||
                GetState() != JSON_STATE_OBJECT_KEY );
    }

    // A helper method for scalars (besides strings) that prevents them
    // from being used as object keys.
    void OnPutValue() {
        CheckNotKey();
        UpdateState();
    }

    // Convenience forwarding methods for sink_.
    void Put ( char c ) {
        sink_->Put ( c );
    }

    void PutN ( const char* str, size_t len ) {
        sink_->PutN ( str, len );
    }

    ByteSinkInterface* sink_;
    std::vector<State> state_;
};

}  // namespace webgl_loader

#endif  // WEBGL_LOADER_JSON_H