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

#ifndef WEBGL_LOADER_BASE_H_
#define WEBGL_LOADER_BASE_H_

#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

// TODO: consider using C99 spellings.
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;

// printf format strings for size_t.
#ifdef _WIN32
# define PRIuS "%Iu"
#else  // Most compilers use the C99 format string.
# define PRIuS "%zu"
#endif

#ifndef isfinite
# define isfinite _finite
#endif

typedef std::vector<float> AttribList;
typedef std::vector<int> IndexList;
typedef std::vector<uint16> QuantizedAttribList;
typedef std::vector<uint16> OptimizedIndexList;

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

static inline int strtoint ( const char* str, const char** endptr ) {
    return static_cast<int> ( strtol ( str, const_cast<char**> ( endptr ), 10 ) );
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

// Jenkin's One-at-a-time Hash. Not the best, but simple and
// portable.
uint32 SimpleHash ( char *key, size_t len, uint32 seed = 0 ) {
    uint32 hash = seed;
    for ( size_t i = 0; i < len; ++i ) {
        hash += static_cast<unsigned char> ( key[i] );
        hash += ( hash << 10 );
        hash ^= ( hash >> 6 );
    }
    hash += ( hash << 3 );
    hash ^= ( hash >> 11 );
    hash += ( hash << 15 );
    return hash;
}

void ToHex ( uint32 w, char out[9] ) {
    const char kOffset0 = '0';
    const char kOffset10 = 'a' - 10;
    out[8] = '\0';
    for ( size_t i = 8; i > 0; ) {
        uint32 bits = w & 0xF;
        out[--i] = bits + ( ( bits < 10 ) ? kOffset0 : kOffset10 );
        w >>= 4;
    }
}

uint16 Quantize ( float f, float in_min, float in_scale, uint16 out_max ) {
    return static_cast<uint16> ( out_max * ( ( f-in_min ) / in_scale ) );
}

// TODO: Visual Studio calls this someting different.
#ifdef putc_unlocked
# define PutChar putc_unlocked
#else
# define PutChar putc
#endif  // putc_unlocked

#ifndef CHECK
# define CHECK(PRED) if (!(PRED)) {                                     \
    fprintf(stderr, "%s:%d CHECK failed: ", __FILE__, __LINE__);        \
    fputs(#PRED "\n", stderr);                                          \
    exit(-1); } else
#endif  // CHECK

#ifndef DCHECK
# ifdef DEBUG
#  define DCHECK(PRED) CHECK(PRED)
# else
#  define DCHECK(PRED)
# endif  // DEBUG
#endif  // DCHECK

#endif  // WEBGL_LOADER_BASE_H_
