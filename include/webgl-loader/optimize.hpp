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

#ifndef WEBGL_LOADER_OPTIMIZE_HPP
#define WEBGL_LOADER_OPTIMIZE_HPP

#include <webgl-loader/base.hpp>

namespace webgl_loader {

// TODO: use vcacne implementation

// TODO: since most vertices are part of 6 faces, you can optimize
// this by using a small inline buffer.
typedef std::vector<int> FaceList;

// Linear-Speed Vertex Cache Optimisation, via:
// http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html
class VertexOptimizer {
private:
    static const int32_t  K_UNKNOWN_INDEX = -1;
    static const uint16_t K_MAX_OUTPUT_INDEX = 0xD800;
    static const size_t   K_CACHE_SIZE = 32; // Does larger improve compression?

    struct VertexData {
        FaceList faces;
        unsigned int cache_tag;  // K_CACHE_SIZE means not in cache.
        float score;
        uint16_t output_index;

        void update_score();
        void remove_face ( int32_t tri );
    };

    const QuantizedAttribList& attribs_;
    std::vector<VertexData> per_vertex_;
    int cache_[K_CACHE_SIZE + 1];
    uint16_t next_unused_index_;

public:
    struct TriangleData {
        bool active;  // true iff triangle has not been optimized and emitted.
        // TODO: eliminate some wasted computation by using this cache.
        // float score;
    };

    VertexOptimizer ( const QuantizedAttribList& attribs );
    void add_triangles ( const int32_t* indices, size_t length, WebGLMeshList* meshes );
private:
    int32_t find_best_triangle ( const int32_t* indices, const std::vector<TriangleData>& per_tri );
    void insert_index_to_cache ( int32_t index );
};

} // namespace webgl_loader

#endif  // WEBGL_LOADER_OPTIMIZE_HPP
