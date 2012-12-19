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

#ifndef WEBGL_LOADER_BOUNDS_HPP
#define WEBGL_LOADER_BOUNDS_HPP

#include <webgl-loader/base.hpp>

namespace webgl_loader {

// TODO: arbitrary vertex formats.

struct Bounds {
    float mins[8];
    float maxes[8];

    void  clear();
    void  enclose_attrib ( const float* attribs );
    void  enclose ( const AttribList& attribs );
    float uniform_scale() const;
};

// TODO: make maxPosition et. al. configurable.
struct BoundsParams {
    float   mins[8];
    float   scales[8];
    int32_t output_maxes[8];
    int32_t decode_offsets[8];
    float   decode_scales[8];

    static BoundsParams from_bounds ( const Bounds& bounds );
    void dump_json ( FILE* out = stdout );
};

} // namespace webgl_loader

#endif  // WEBGL_LOADER_BOUNDS_HPP
