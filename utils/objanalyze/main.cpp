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

#include <webgl-loader/bounds.hpp>
#include <webgl-loader/mesh.hpp>
#include <webgl-loader/optimize.hpp>

// Return cache misses from a simulated FIFO cache.
template <typename IndexListT>
size_t CountFifoCacheMisses ( const IndexListT& indices, const size_t cache_size ) {
    static const size_t kMaxCacheSize = 32;
    static const int kUnknownIndex = -1;
    assert ( cache_size <= kMaxCacheSize );
    int fifo[kMaxCacheSize + 1];
    for ( size_t i = 0; i < cache_size; ++i ) {
        fifo[i] = kUnknownIndex;
    }
    size_t misses = 0;
    for ( size_t i = 0; i < indices.size(); ++i ) {
        const int idx = indices[i];
        // Use a sentry to simplify the FIFO search.
        fifo[cache_size] = idx;
        size_t at = 0;
        while ( fifo[at] != idx ) ++at;
        if ( at == cache_size ) {
            ++misses;
            int write_idx = idx;
            for ( size_t j = 0; j < cache_size; ++j ) {
                const int swap_idx = fifo[j];
                fifo[j] = write_idx;
                write_idx = swap_idx;
            }
        }
    }
    return misses;
}

template <typename IndexListT>
void PrintCacheAnalysisRow ( const IndexListT& indices, const size_t cache_size,
                             const size_t num_verts, const size_t num_tris ) {
    const size_t misses = CountFifoCacheMisses ( indices, cache_size );
    const double misses_as_double = static_cast<double> ( misses );
    printf ( "||" PRIuS "||" PRIuS "||%f||%f||\n", cache_size, misses,
             misses_as_double / num_verts, misses_as_double / num_tris );
}

template <typename IndexListT>
void PrintCacheAnalysisTable ( const size_t count, const char** args,
                               const IndexListT& indices,
                               const size_t num_verts, const size_t num_tris ) {
    printf ( PRIuS " vertices, " PRIuS " triangles\n\n", num_verts, num_tris );
    puts ( "||Cache Size||# misses||ATVR||ACMR||" );
    for ( size_t i = 0; i < count; ++i ) {
        int cache_size = atoi ( args[i] );
        if ( cache_size > 1 ) {
            PrintCacheAnalysisRow ( indices, cache_size, num_verts, num_tris );
        }
    }
}

int main ( int argc, const char* argv[] ) {
    if ( argc < 2 ) {
        fprintf ( stderr, "Usage: %s in.obj [list of cache sizes]\n\n"
                  "\tPerform vertex cache analysis on in.obj using specified sizes.\n"
                  "\tFor example: %s in.obj 6 16 24 32\n"
                  "\tMaximum cache size is 32.\n\n",
                  argv[0], argv[0] );
        return -1;
    }
    FILE* fp = fopen ( argv[1], "r" );
    WavefrontObjFile obj ( fp );
    fclose ( fp );
    std::vector<DrawMesh> meshes;
    obj.CreateDrawMeshes ( &meshes );
    const DrawMesh& draw_mesh = meshes[0];

    size_t count = 4;
    const char* default_args[] = { "6", "16", "24", "32" };
    const char** args = &default_args[0];
    if ( argc > 2 ) {
        count = argc - 1;
        args = argv + 1;
    }

    puts ( "\nBefore:\n" );
    PrintCacheAnalysisTable ( count, args, draw_mesh.indices,
                              draw_mesh.attribs.size() / 8,
                              draw_mesh.indices.size() / 3 );

    QuantizedAttribList attribs;
    BoundsParams bounds_params;
    AttribsToQuantizedAttribs ( meshes[0].attribs, &bounds_params, &attribs );
    VertexOptimizer vertex_optimizer ( attribs, meshes[0].indices );
    WebGLMeshList webgl_meshes;
    vertex_optimizer.GetOptimizedMeshes ( &webgl_meshes );
    for ( size_t i = 0; i < webgl_meshes.size(); ++i ) {
        puts ( "\nAfter:\n" );
        PrintCacheAnalysisTable ( count, args, webgl_meshes[i].indices,
                                  webgl_meshes[i].attribs.size() / 8,
                                  webgl_meshes[i].indices.size() / 3 );
    }
    return 0;
}
