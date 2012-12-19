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

#ifndef WEBGL_LOADER_BASE_HPP
#define WEBGL_LOADER_BASE_HPP

#include <cmath>
#include <cctype>
#include <limits>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#define PRIuS "%zu"

namespace webgl_loader {

typedef std::vector<float> AttribList;
typedef std::vector<int32_t> IndexList;
typedef std::vector<uint16_t> QuantizedAttribList;
typedef std::vector<uint16_t> OptimizedIndexList;

// TODO: these data structures ought to go elsewhere.
struct DrawMesh {
    // Interleaved vertex format:
    //  3-D Position
    //  3-D Normal
    //  2-D TexCoord
    // Note that these
    AttribList attribs;
    // Indices are 0-indexed.
    IndexList indices;
};

struct WebGLMesh {
    QuantizedAttribList attribs;
    OptimizedIndexList indices;
};

typedef std::vector<WebGLMesh> WebGLMeshList;

static inline int32_t strtoint ( const char* str, const char** endptr ) {
    return static_cast<int32_t> ( strtol ( str, const_cast<char**> ( endptr ), 10 ) );
}

static inline const char* StripLeadingWhitespace ( const char* str ) {
    while ( isspace ( *str ) ) {
        ++str;
    }
    return str;
}

static inline char* StripLeadingWhitespace ( char* str ) {
    while ( isspace ( *str ) ) {
        ++str;
    }
    return str;
}

// Like basename.
static inline const char* StripLeadingDir ( const char* const str ) {
    const char* last_slash = NULL;
    const char* pos = str;
    while ( const char ch = *pos ) {
        if ( ch == '/' || ch == '\\' ) {
            last_slash = pos;
        }
        ++pos;
    }
    return last_slash ? ( last_slash + 1 ) : str;
}

static inline void TerminateAtNewlineOrComment ( char* str ) {
    char* newline = strpbrk ( str, "#\r\n" );
    if ( newline ) {
        *newline = '\0';
    }
}

static inline const char* ConsumeFirstToken ( const char* const line,
        std::string* token ) {
    const char* curr = line;
    while ( char ch = *curr ) {
        if ( isspace ( ch ) ) {
            token->assign ( line, curr );
            return curr + 1;
        }
        ++curr;
    }
    if ( curr == line ) {
        return NULL;
    }
    token->assign ( line, curr );
    return curr;
}

static inline void ToLower ( const char* in, std::string* out ) {
    while ( char ch = *in ) {
        out->push_back ( tolower ( ch ) );
        ++in;
    }
}

static inline void ToLowerInplace ( std::string* in ) {
    std::string& s = *in;
    for ( size_t i = 0; i < s.size(); ++i ) {
        s[i] = tolower ( s[i] );
    }
}

uint32_t SimpleHash ( char *key, size_t len, uint32_t seed = 0 );
void ToHex ( uint32_t w, char out[9] );
uint16_t Quantize ( float f, float in_min, float in_scale, uint16_t out_max );

} // namespace webgl_loader

#endif  // WEBGL_LOADER_BASE_HPP
