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

#ifndef WEBGL_LOADER_BOUNDS_H_
#define WEBGL_LOADER_BOUNDS_H_

#include <stdio.h>

#include <webgl-loader/base.h>

namespace webgl_loader {

// TODO: arbitrary vertex formats.

struct Bounds {
    float mins[8];
    float maxes[8];

    void Clear() {
        for ( size_t i = 0; i < 8; ++i ) {
            mins[i] = FLT_MAX;
            maxes[i] = -FLT_MAX;
        }
    }

    void EncloseAttrib ( const float* attribs ) {
        for ( size_t i = 0; i < 8; ++i ) {
            const float attrib = attribs[i];
            if ( mins[i] > attrib ) {
                mins[i] = attrib;
            }
            if ( maxes[i] < attrib ) {
                maxes[i] = attrib;
            }
        }
    }

    void Enclose ( const AttribList& attribs ) {
        for ( size_t i = 0; i < attribs.size(); i += 8 ) {
            EncloseAttrib ( &attribs[i] );
        }
    }

    float UniformScale() const {
        const float x = maxes[0] - mins[0];
        const float y = maxes[1] - mins[1];
        const float z = maxes[2] - mins[2];
        return ( x > y ) // TODO: max3
               ? ( ( x > z ) ? x : z )
                   : ( ( y > z ) ? y : z );
    }
};

// TODO: make maxPosition et. al. configurable.
struct BoundsParams {
    static BoundsParams FromBounds ( const Bounds& bounds ) {
        BoundsParams ret;
        const float scale = bounds.UniformScale();
        // Position. Use a uniform scale.
        for ( size_t i = 0; i < 3; ++i ) {
            const int maxPosition = ( 1 << 14 ) - 1; // 16383;
            ret.mins[i] = bounds.mins[i];
            ret.scales[i] = scale;
            ret.outputMaxes[i] = maxPosition;
            ret.decodeOffsets[i] = maxPosition * bounds.mins[i] / scale;
            ret.decodeScales[i] = scale / maxPosition;
        }
        // TexCoord.
        // TODO: get bounds-dependent texcoords working!
        for ( size_t i = 3; i < 5; ++i ) {
            // const float texScale = bounds.maxes[i] - bounds.mins[i];
            const int maxTexcoord = ( 1 << 10 ) - 1; // 1023
            ret.mins[i] = 0;  //bounds.mins[i];
            ret.scales[i] = 1;  //texScale;
            ret.outputMaxes[i] = maxTexcoord;
            ret.decodeOffsets[i] = 0;  //maxTexcoord * bounds.mins[i] / texScale;
            ret.decodeScales[i] = 1.0f / maxTexcoord;  // texScale / maxTexcoord;
        }
        // Normal. Always uniform range.
        for ( size_t i = 5; i < 8; ++i ) {
            ret.mins[i] = -1;
            ret.scales[i] = 2.f;
            ret.outputMaxes[i] = ( 1 << 10 ) - 1; // 1023
            ret.decodeOffsets[i] = 1 - ( 1 << 9 ); // -511
            ret.decodeScales[i] = 1.0 / 511;
        }
        return ret;
    }

    void DumpJson ( FILE* out = stdout ) {
        // TODO: use JsonSink.
        fputs ( "{\n", out );
        fprintf ( out, "    \"decodeOffsets\": [%d,%d,%d,%d,%d,%d,%d,%d],\n",
                  decodeOffsets[0], decodeOffsets[1], decodeOffsets[2],
                  decodeOffsets[3], decodeOffsets[4], decodeOffsets[5],
                  decodeOffsets[6], decodeOffsets[7] );
        fprintf ( out, "    \"decodeScales\": [%f,%f,%f,%f,%f,%f,%f,%f]\n",
                  decodeScales[0], decodeScales[1], decodeScales[2], decodeScales[3],
                  decodeScales[4], decodeScales[5], decodeScales[6], decodeScales[7] );
        fputs ( "  }", out );
    }

    float mins[8];
    float scales[8];
    int outputMaxes[8];
    int decodeOffsets[8];
    float decodeScales[8];
};

}  // namespace webgl_loader

#endif  // WEBGL_LOADER_BOUNDS_H_
