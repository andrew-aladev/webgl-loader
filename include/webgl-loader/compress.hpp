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

#ifndef WEBGL_LOADER_COMPRESS_HPP
#define WEBGL_LOADER_COMPRESS_HPP

#include <webgl-loader/bounds.hpp>
#include <webgl-loader/utf8.hpp>

namespace webgl_loader {

inline uint16_t zig_zag ( int16_t word ) {
    return ( word >> 15 ) ^ ( word << 1 );
}

void attribs_to_quantized_attribs ( const AttribList& interleaved_attribs, const BoundsParams& bounds_params, QuantizedAttribList* quantized_attribs );
void compress_aabb_to_utf8 ( const Bounds& bounds, const BoundsParams& total_bounds, ByteSinkInterface* utf8 );
void compress_indices_to_utf8 ( const OptimizedIndexList& list, ByteSinkInterface* utf8 );
void compress_quantized_attribs_to_utf8 ( const QuantizedAttribList& attribs, ByteSinkInterface* utf8 );

class EdgeCachingCompressor {
public:
    // Assuming that the vertex cache optimizer LRU is 32 vertices, we
    // expect ~64 triangles, and ~96 edges.
    static const size_t  K_MAX_LRU_SIZE = 96;
    static const int32_t K_LRU_SENTINEL = -1;

private:
    // |attribs_| and |indices_| is the input mesh.
    const QuantizedAttribList& attribs_;
    // |indices_| are non-const because |compress| may update triangle
    // winding order.
    OptimizedIndexList& indices_;
    // |deltas_| contains the compressed attributes. They can be
    // compressed in one of two ways:
    // (1) with parallelogram prediction, compared with the predicted vertex,
    // (2) otherwise, compared with the last referenced vertex.
    // Note that even (2) is different and probably better than what
    // |compress_quantized_attribs_to_utf8| does, which is comparing with
    // the last encoded vertex.
    QuantizedAttribList deltas_;
    // |codes_| contains the compressed indices. Small values encode an
    // edge match; that is, the first edge of the next triangle matches
    // a recently-seen edge.
    OptimizedIndexList codes_;
    // |index_high_water_mark_| is used as it is in |compress_indices_to_utf8|.
    uint16_t index_high_water_mark_;
    // |last_attrib_referenced_| is the index of the last referenced
    // attribute. This is used to delta encode attributes when no edge match
    // is found.
    uint16_t last_attrib_[8];
    size_t lru_size_;
    // |edge_lru_| contains the LRU lits of edge references. It stores
    // indices to the input |indices_|. By convention, an edge points to
    // the vertex opposite the edge in question. We pad the array by a
    // triangle to simplify edge cases.
    int32_t edge_lru_[K_MAX_LRU_SIZE + 3];

public:
    EdgeCachingCompressor ( const QuantizedAttribList& attribs, OptimizedIndexList& indices );

    void compress_with_lru ( ByteSinkInterface* utf8 );
    void compress ( ByteSinkInterface* utf8 );

    const QuantizedAttribList& get_deltas() const;
    const OptimizedIndexList&  get_codes()  const;

    void dump_debug ( FILE* fp = stdout );

private:
    void simple_predictor ( size_t max_backref, size_t triangle_start_index );
    void parallelogram_predictor ( uint16_t backref_edge, size_t backref_vert, size_t triangle_start_index );
    
    bool highwater_mark ( uint16_t index, uint16_t start_code = 0 );
    
    void encode_delta_attrib ( size_t index, const uint16_t* predicted );
    void update_last_attrib ( uint16_t index );
    
    size_t lru_edge ( const uint16_t* triangle, size_t* match_indices, size_t* match_winding );
    void   lru_edge_zero ( const uint16_t* triangle );
    void   lru_edge_one ( size_t i0, size_t i1, size_t match_index );
    void   lru_edge_two ( int32_t i0, size_t match_index0, size_t match_index1 );
    void   lru_edge_three ( size_t match_index0, size_t match_index1, size_t match_index2 );
};

}  // namespace webgl_loader

#endif  // WEBGL_LOADER_COMPRESS_HPP
