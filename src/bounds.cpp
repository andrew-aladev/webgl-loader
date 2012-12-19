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

#include <webgl-loader/bounds.hpp>

namespace webgl_loader {

void Bounds::clear() {
    for ( size_t i = 0; i < 8; ++i ) {
        mins[i]  =   std::numeric_limits<float>::max();
        maxes[i] = - std::numeric_limits<float>::max();
    }
}

void Bounds::enclose_attrib ( const float* attribs ) {
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

void Bounds::enclose ( const AttribList& attribs ) {
    for ( size_t i = 0; i < attribs.size(); i += 8 ) {
        enclose_attrib ( &attribs[i] );
    }
}

float Bounds::uniform_scale() const {
    const float x = maxes[0] - mins[0];
    const float y = maxes[1] - mins[1];
    const float z = maxes[2] - mins[2];

    // TODO: max3
    if ( x > y ) {
        if ( x > z ) {
            return x;
        } else {
            return z;
        }
    } else {
        if ( y > z ) {
            return y;
        } else {
            return z;
        }
    }
}

BoundsParams BoundsParams::from_bounds ( const Bounds& bounds ) {
    BoundsParams result;
    const float scale = bounds.uniform_scale();

    // Position. Use a uniform scale.
    for ( size_t i = 0; i < 3; ++i ) {
        const int32_t max_position = ( 1 << 14 ) - 1; // 16383;
        result.mins[i]   = bounds.mins[i];
        result.scales[i] = scale;
        result.output_maxes[i]   = max_position;
        result.decode_offsets[i] = max_position * bounds.mins[i] / scale;
        result.decode_scales[i]  = scale / max_position;
    }

    // TexCoord.
    // TODO: get bounds-dependent texcoords working!
    for ( size_t i = 3; i < 5; ++i ) {
        // const float tex_scale = bounds.maxes[i] - bounds.mins[i];
        const int32_t max_texcoord = ( 1 << 10 ) - 1; // 1023
        result.mins[i]   = 0;  //bounds.mins[i];
        result.scales[i] = 1;  //tex_scale;
        result.output_maxes[i]   = max_texcoord;
        result.decode_offsets[i] = 0;  //max_texcoord * bounds.mins[i] / tex_scale;
        result.decode_scales[i]  = 1.0f / max_texcoord;  // tex_scale / max_texcoord;
    }

    // Normal. Always uniform range.
    for ( size_t i = 5; i < 8; ++i ) {
        result.mins[i]   = -1;
        result.scales[i] = 2.f;
        result.output_maxes[i]   = ( 1 << 10 ) - 1; // 1023
        result.decode_offsets[i] = 1 - ( 1 << 9 ); // -511
        result.decode_scales[i]  = 1.0 / 511;
    }

    return result;
}

void BoundsParams::dump_json ( FILE* out ) {
    // TODO: use JsonSink.
    fputs ( "{\n", out );
    fprintf ( out, "    \"decode_offsets\": [%d,%d,%d,%d,%d,%d,%d,%d],\n",
              decode_offsets[0], decode_offsets[1], decode_offsets[2],
              decode_offsets[3], decode_offsets[4], decode_offsets[5],
              decode_offsets[6], decode_offsets[7] );
    fprintf ( out, "    \"decode_scales\": [%f,%f,%f,%f,%f,%f,%f,%f]\n",
              decode_scales[0], decode_scales[1], decode_scales[2], decode_scales[3],
              decode_scales[4], decode_scales[5], decode_scales[6], decode_scales[7] );
    fputs ( "  }", out );
}

} // namespace webgl_loader
