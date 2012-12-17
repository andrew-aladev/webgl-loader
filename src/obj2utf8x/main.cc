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

#include <webgl-loader/bounds.h>
#include <webgl-loader/compress.h>
#include <webgl-loader/mesh.h>
#include <webgl-loader/optimize.h>
#include <webgl-loader/stream.h>

int main ( int argc, const char* argv[] ) {
    FILE* json_out = stdout;
    if ( argc != 3 && argc != 4 ) {
        fprintf ( stderr, "Usage: %s in.obj out.utf8\n\n"
                  "\tCompress in.obj to out.utf8 and writes JS to STDOUT.\n\n",
                  argv[0] );
        return -1;
    } else if ( argc == 4 ) {
        json_out = fopen ( argv[3], "w" );
        assert ( json_out != NULL );
    }

    FILE* fp = fopen ( argv[1], "r" );
    WavefrontObjFile obj ( fp );
    fclose ( fp );

    fputs ( "{\n  \"materials\": {\n", json_out );
    const MaterialList& materials = obj.materials();
    for ( size_t i = 0; i < materials.size(); ++i ) {
        materials[i].DumpJson ( json_out );
        const bool last = i == materials.size() - 1;
        fputs ( ",\n" + last, json_out );
    }
    fputs ( "  },\n", json_out );

    const MaterialBatches& batches = obj.material_batches();

    // Pass 1: compute bounds.
    webgl_loader::Bounds bounds;
    bounds.Clear();
    for ( MaterialBatches::const_iterator iter = batches.begin();
            iter != batches.end(); ++iter ) {
        const DrawBatch& draw_batch = iter->second;
        bounds.Enclose ( draw_batch.draw_mesh().attribs );
    }
    webgl_loader::BoundsParams bounds_params =
        webgl_loader::BoundsParams::FromBounds ( bounds );
    fputs ( "  \"decodeParams\": ", json_out );
    bounds_params.DumpJson ( json_out );
    fputs ( ",\n  \"urls\": {\n", json_out );
    // Pass 2: quantize, optimize, compress, report.
    FILE* utf8_out_fp = fopen ( argv[2], "wb" );
    assert ( utf8_out_fp != NULL );
    fprintf ( json_out, "    \"%s\": [\n", argv[2] );
    webgl_loader::FileSink utf8_sink ( utf8_out_fp );
    size_t offset = 0;
    MaterialBatches::const_iterator iter = batches.begin();
    while ( iter != batches.end() ) {
        const DrawMesh& draw_mesh = iter->second.draw_mesh();
        if ( draw_mesh.indices.empty() ) {
            ++iter;
            continue;
        }
        QuantizedAttribList quantized_attribs;
        webgl_loader::AttribsToQuantizedAttribs ( draw_mesh.attribs, bounds_params,
                &quantized_attribs );
        VertexOptimizer vertex_optimizer ( quantized_attribs );
        const std::vector<GroupStart>& group_starts = iter->second.group_starts();
        WebGLMeshList webgl_meshes;
        std::vector<size_t> group_lengths;
        for ( size_t i = 1; i < group_starts.size(); ++i ) {
            const size_t here = group_starts[i-1].offset;
            const size_t length = group_starts[i].offset - here;
            group_lengths.push_back ( length );
            vertex_optimizer.AddTriangles ( &draw_mesh.indices[here], length,
                                            &webgl_meshes );
        }
        const size_t here = group_starts.back().offset;
        const size_t length = draw_mesh.indices.size() - here;
        assert ( length % 3 == 0 );
        group_lengths.push_back ( length );
        vertex_optimizer.AddTriangles ( &draw_mesh.indices[here], length,
                                        &webgl_meshes );

        std::vector<std::string> material;
        // TODO: is this buffering still necessary?
        std::vector<size_t> attrib_start, attrib_length,
            code_start, code_length, num_tris;
        for ( size_t i = 0; i < webgl_meshes.size(); ++i ) {
            const size_t num_attribs = webgl_meshes[i].attribs.size();
            const size_t num_indices = webgl_meshes[i].indices.size();
            assert ( num_attribs % 8 == 0 );
            assert ( num_indices % 3 == 0 );
            webgl_loader::EdgeCachingCompressor compressor ( webgl_meshes[i].attribs,
                    webgl_meshes[i].indices );
            compressor.Compress ( &utf8_sink );
            material.push_back ( iter->first );
            attrib_start.push_back ( offset );
            attrib_length.push_back ( num_attribs / 8 );
            code_start.push_back ( offset + num_attribs );
            code_length.push_back ( compressor.codes().size() );
            num_tris.push_back ( num_indices / 3 );
            offset += num_attribs + compressor.codes().size();
        }
        for ( size_t i = 0; i < webgl_meshes.size(); ++i ) {
            fprintf ( json_out,
                      "      { \"material\": \"%s\",\n"
                      "        \"attribRange\": [" PRIuS ", " PRIuS "],\n"
                      "        \"codeRange\": [" PRIuS ", " PRIuS ", " PRIuS "]\n"
                      "      }",
                      material[i].c_str(),
                      attrib_start[i], attrib_length[i],
                      code_start[i], code_length[i], num_tris[i] );
            if ( i != webgl_meshes.size() - 1 ) {
                fputs ( ",\n", json_out );
            }
        }
        const bool last = ( ++iter == batches.end() );
        fputs ( ",\n" + last, json_out );
    }
    fputs ( "    ]\n", json_out );
    fputs ( "  }\n}", json_out );
    return 0;
}
