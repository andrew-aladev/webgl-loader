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
//   Google Inc (wonchun@gmail.com)
//   andrew.aladjev@gmail.com

#include <webgl-loader/bounds.hpp>
#include <webgl-loader/compress.hpp>
#include <webgl-loader/mesh.hpp>
#include <webgl-loader/optimize.hpp>
#include <webgl-loader/stream.hpp>

using namespace webgl_loader;

int main ( int argc, const char* argv[] ) {
    if ( argc != 3 ) {
        fprintf ( stderr, "Usage: %s in.obj out.utf8\n\n"
                  "\tCompress in.obj to out.utf8 and writes JS to STDOUT.\n\n",
                  argv[0] );
        return -1;
    }
    FILE* fp = fopen ( argv[1], "r" );
    WavefrontObjFile obj ( fp );
    fclose ( fp );

    printf ( "MODELS[\'%s\'] = {\n", StripLeadingDir ( argv[1] ) );
    puts ( "  materials: {" );
    const MaterialList& materials = obj.materials();
    for ( size_t i = 0; i < materials.size(); ++i ) {
        materials[i].dump_json();
    }
    puts ( "  }," );

    const MaterialBatches& batches = obj.material_batches();

    // Pass 1: compute bounds.
    Bounds bounds;
    bounds.clear();
    for ( MaterialBatches::const_iterator iter = batches.begin();
            iter != batches.end(); ++iter ) {
        const DrawBatch& draw_batch = iter->second;
        bounds.enclose ( draw_batch.draw_mesh().attribs );
    }
    BoundsParams bounds_params =
        BoundsParams::from_bounds ( bounds );
    printf ( "  decodeParams: " );
    bounds_params.dump_json();

    puts ( "  urls: {" );
    std::vector<char> utf8;
    VectorSink sink ( &utf8 );
    // Pass 2: quantize, optimize, compress, report.
    for ( MaterialBatches::const_iterator iter = batches.begin();
            iter != batches.end(); ++iter ) {
        size_t offset = 0;
        utf8.clear();
        const DrawMesh& draw_mesh = iter->second.draw_mesh();
        if ( draw_mesh.indices.empty() ) continue;

        QuantizedAttribList quantized_attribs;
        attribs_to_quantized_attribs ( draw_mesh.attribs, bounds_params,
                &quantized_attribs );
        VertexOptimizer vertex_optimizer ( quantized_attribs );
        const std::vector<GroupStart>& group_starts = iter->second.group_starts();
        WebGLMeshList webgl_meshes;
        std::vector<size_t> group_lengths;
        for ( size_t i = 1; i < group_starts.size(); ++i ) {
            const size_t here = group_starts[i-1].offset;
            const size_t length = group_starts[i].offset - here;
            group_lengths.push_back ( length );
            vertex_optimizer.add_triangles ( &draw_mesh.indices[here], length,
                                            &webgl_meshes );
        }
        const size_t here = group_starts.back().offset;
        const size_t length = draw_mesh.indices.size() - here;
        const bool divisible_by_3 = length % 3 == 0;
        assert ( divisible_by_3 );
        group_lengths.push_back ( length );
        vertex_optimizer.add_triangles ( &draw_mesh.indices[here], length,
                                        &webgl_meshes );

        std::vector<std::string> material;
        std::vector<size_t> attrib_start, attrib_length, index_start, index_length;
        for ( size_t i = 0; i < webgl_meshes.size(); ++i ) {
            const size_t num_attribs = webgl_meshes[i].attribs.size();
            const size_t num_indices = webgl_meshes[i].indices.size();
            const bool kBadSizes = num_attribs % 8 || num_indices % 3;
            assert ( !kBadSizes );
            compress_quantized_attribs_to_utf8 ( webgl_meshes[i].attribs,
                    &sink );
            compress_indices_to_utf8 ( webgl_meshes[i].indices, &sink );
            material.push_back ( iter->first );
            attrib_start.push_back ( offset );
            attrib_length.push_back ( num_attribs / 8 );
            index_start.push_back ( offset + num_attribs );
            index_length.push_back ( num_indices / 3 );
            offset += num_attribs + num_indices;
        }
        const uint32_t hash = SimpleHash ( &utf8[0], utf8.size() );
        char buf[9] = { '\0' };
        ToHex ( hash, buf );
        // TODO: this needs to handle paths.
        std::string out_fn = std::string ( buf ) + "." + argv[2];
        FILE* out_fp = fopen ( out_fn.c_str(), "wb" );
        printf ( "    \'%s\': [\n", out_fn.c_str() );
        size_t group_index = 0;
        for ( size_t i = 0; i < webgl_meshes.size(); ++i ) {
            printf ( "      { material: \'%s\',\n"
                     "        attribRange: [" PRIuS ", " PRIuS "],\n"
                     "        indexRange: [" PRIuS ", " PRIuS "],\n"
                     "        bboxes: " PRIuS ",\n"
                     "        names: [",
                     material[i].c_str(),
                     attrib_start[i], attrib_length[i],
                     index_start[i], index_length[i],
                     offset );
            std::vector<size_t> buffered_lengths;
            size_t group_start = 0;
            while ( group_index < group_lengths.size() ) {
                printf ( "\'%s\', ",
                         obj.line_to_group ( group_starts[group_index].group_line ).c_str() );
                const size_t group_length = group_lengths[group_index];
                const size_t next_start = group_start + group_length;
                const size_t webgl_index_length = webgl_meshes[i].indices.size();
                // TODO: bbox info is better placed at the head of the file,
                // perhaps transposed. Also, when a group gets split between
                // batches, the bbox gets stored twice.
                compress_aabb_to_utf8 ( group_starts[group_index].bounds,
                                                   bounds_params, &sink );
                offset += 6;
                if ( next_start < webgl_index_length ) {
                    buffered_lengths.push_back ( group_length );
                    group_start = next_start;
                    ++group_index;
                } else {
                    const size_t fits = webgl_index_length - group_start;
                    buffered_lengths.push_back ( fits );
                    group_start = 0;
                    group_lengths[group_index] -= fits;
                    break;
                }
            }
            printf ( "],\n        lengths: [" );
            for ( size_t k = 0; k < buffered_lengths.size(); ++k ) {
                printf ( PRIuS ", ", buffered_lengths[k] );
            }
            puts ( "],\n      }," );
        }
        fwrite ( &utf8[0], 1, utf8.size(), out_fp );
        fclose ( out_fp );
        puts ( "    ]," );
    }
    puts ( "  }\n};" );
    return 0;
}
