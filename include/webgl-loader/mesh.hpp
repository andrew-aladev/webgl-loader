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

#ifndef WEBGL_LOADER_MESH_HPP
#define WEBGL_LOADER_MESH_HPP

#include <map>
#include <utility>

#include <webgl-loader/bounds.hpp>
#include <webgl-loader/utf8.hpp>

namespace webgl_loader {

// A short list of floats, useful for parsing a single vector
// attribute.
class ShortFloatList {
public:
    // MeshLab can create position attributes with
    // color coordinates like: v x y z r g b
    static const size_t K_MAX_NUM_FLOATS = 6;

private:
    float a_[K_MAX_NUM_FLOATS];
    size_t size_;

public:
    ShortFloatList();
    void clear();
    size_t parse_line ( const char* line );
    float operator[] ( size_t idx ) const;
    void append_to ( AttribList* attribs ) const;
    void append_n_to ( AttribList* attribs, const size_t sz ) const;
    bool empty() const;
    size_t size() const;
};

class IndexFlattener {
private:
    static const int32_t K_INDEX_UNKNOWN      = -1;
    static const int32_t K_INDEX_NOT_IN_TABLE = -2;

    struct IndexType {
        // I'm being tricky/lazy here. The table_ stores the flattened index in the first field, since it is indexed by position.
        // The map_ stores position and uses this struct as a key to lookup the flattened index.
        int32_t position_or_flat;
        int32_t texcoord;
        int32_t normal;

        IndexType();
        IndexType ( int32_t position_index, int32_t texcoord_index, int32_t normal_index );

        bool operator< ( const IndexType& that ) const;
        bool operator== ( const IndexType& that ) const;
        bool operator!= ( const IndexType& that ) const;
    };

    typedef std::map<IndexType, int32_t> MapType;

    std::vector<IndexType> table_;
    MapType map_;
    int32_t count_;

public:
    explicit IndexFlattener ( size_t num_positions );
    int32_t count() const;
    void reserve ( size_t size );
    std::pair<int32_t, bool> get_flattened_index ( int32_t position_index, int32_t texcoord_index, int32_t normal_index );

private:
    std::pair<int32_t, bool> get_flattened_index_from_map ( int32_t position_index, int32_t texcoord_index, int32_t normal_index );
};

static inline size_t position_dim() {
    return 3;
}
static inline size_t texcoord_dim() {
    return 2;
}
static inline size_t normal_dim() {
    return 3;
}

// TODO: Make a c'tor to properly initialize.
struct GroupStart {
    size_t offset;  // offset into draw_mesh_.indices.
    uint32_t group_line;
    int32_t min_index, max_index;  // range into attribs.
    Bounds bounds;
};

class DrawBatch {
private:
    AttribList* positions_, *texcoords_, *normals_;
    DrawMesh draw_mesh_;
    IndexFlattener flattener_;
    unsigned int current_group_line_;
    std::vector<GroupStart> group_starts_;

public:
    DrawBatch();
    const std::vector<GroupStart>& group_starts() const;
    void init ( AttribList* positions, AttribList* texcoords, AttribList* normals );
    void add_triangle ( uint32_t group_line, int32_t* indices );
    const DrawMesh& draw_mesh() const;
};

struct Material {
    std::string name;
    float kd[3];
    std::string map_kd;

    void dump_json ( FILE* out = stdout ) const;
};

typedef std::vector<Material> MaterialList;

class WavefrontMtlFile {
private:
    Material* current_;
    MaterialList materials_;

public:
    explicit WavefrontMtlFile ( FILE* fp );
    const MaterialList& materials() const;

private:
    void parse_file ( FILE* fp );
    void parse_line ( const char* line, uint32_t line_num );
    void parse_color ( const char* line, uint32_t line_num );
    void parse_map_kd ( const char* line, uint32_t line_num );
    void parse_new_mtl ( const char* line, uint32_t line_num );
};

typedef std::map<std::string, DrawBatch> MaterialBatches;

// TODO: consider splitting this into a low-level parser and a high-level object.
class WavefrontObjFile {
private:
    AttribList positions_;
    AttribList texcoords_;
    AttribList normals_;
    MaterialList materials_;

    // Currently, batch by texture (i.e. map_kd).
    MaterialBatches material_batches_;
    DrawBatch* current_batch_;

    typedef std::multimap<uint32_t, std::string> LineToGroups;
    LineToGroups line_to_groups_;
    std::map<std::string, int32_t> group_counts_;
    uint32_t current_group_line_;

public:
    explicit WavefrontObjFile ( FILE* fp );
    const MaterialList& materials() const;
    const MaterialBatches& material_batches() const;
    const std::string& line_to_group ( uint32_t line ) const;
    void dump_debug() const;

private:
    WavefrontObjFile() { }  // For testing.

    void parse_file ( FILE* fp );
    void parse_line ( const char* line, uint32_t line_num );
    void parse_attrib ( const char* line, uint32_t line_num );
    void parse_position ( const ShortFloatList& floats, uint32_t line_num );
    void parse_tex_coord ( const ShortFloatList& floats, uint32_t line_num );
    void parse_normal ( const ShortFloatList& floats, uint32_t line_num );
    void parse_face ( const char* line, uint32_t line_num );
    const char* parse_indices ( const char* line, uint32_t line_num, int32_t* position_index, int32_t* texcoord_index, int32_t* normal_index );
    void parse_group ( const char* line, uint32_t line_num );
    void parse_smoothing_group ( const char* line, uint32_t line_num );
    void parse_mtl_lib ( const char* line, uint32_t line_num );
    void parse_use_mtl ( const char* line, uint32_t line_num );
    void warn_line ( const char* why, uint32_t line_num ) const;
    void error_line ( const char* why, uint32_t line_num ) const;
};

} // namespace webgl_loader

#endif  // WEBGL_LOADER_MESH_HPP
