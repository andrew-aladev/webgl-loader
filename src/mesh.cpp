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

#include <webgl-loader/mesh.hpp>

namespace webgl_loader {

ShortFloatList::ShortFloatList()
    : size_ ( 0 ) {
    clear();
}

void ShortFloatList::clear() {
    for ( size_t i = 0; i < K_MAX_NUM_FLOATS; ++i ) {
        a_[i] = 0.f;
    }
}

// Parse up to K_MAX_NUM_FLOATS from C string.
// TODO: this should instead return endptr, since size is recoverable.
size_t ShortFloatList::parse_line ( const char* line ) {
    for ( size_ = 0; size_ != K_MAX_NUM_FLOATS; ++size_ ) {
        char* endptr = NULL;
        a_[size_] = strtof ( line, &endptr );
        if ( endptr == NULL || line == endptr ) break;
        line = endptr;
    }
    return size_;
}

float ShortFloatList::operator[] ( size_t idx ) const {
    return a_[idx];
}

void ShortFloatList::append_to ( AttribList* attribs ) const {
    append_n_to ( attribs, size_ );
}

void ShortFloatList::append_n_to ( AttribList* attribs, const size_t sz ) const {
    attribs->insert ( attribs->end(), a_, a_ + sz );
}

bool ShortFloatList::empty() const {
    return size_ == 0;
}

size_t ShortFloatList::size() const {
    return size_;
}

IndexFlattener::IndexType::IndexType()
    : position_or_flat ( IndexFlattener::K_INDEX_UNKNOWN ),
      texcoord ( IndexFlattener::K_INDEX_UNKNOWN ),
      normal ( IndexFlattener::K_INDEX_UNKNOWN )
{ }

IndexFlattener::IndexType::IndexType ( int32_t position_index, int32_t texcoord_index, int32_t normal_index )
    : position_or_flat ( position_index ),
      texcoord ( texcoord_index ),
      normal ( normal_index )
{ }

// An ordering for std::map.
bool IndexFlattener::IndexType::operator< ( const IndexFlattener::IndexType& that ) const {
    if ( position_or_flat == that.position_or_flat ) {
        if ( texcoord == that.texcoord ) {
            return normal < that.normal;
        } else {
            return texcoord < that.texcoord;
        }
    } else {
        return position_or_flat < that.position_or_flat;
    }
}

bool IndexFlattener::IndexType::operator== ( const IndexFlattener::IndexType& that ) const {
    return position_or_flat == that.position_or_flat &&
           texcoord == that.texcoord && normal == that.normal;
}

bool IndexFlattener::IndexType::operator!= ( const IndexFlattener::IndexType& that ) const {
    return !operator== ( that );
}

IndexFlattener::IndexFlattener ( size_t num_positions )
    : count_ ( 0 ),
      table_ ( num_positions ) {
}

int32_t IndexFlattener::count() const {
    return count_;
}

void IndexFlattener::reserve ( size_t size ) {
    table_.reserve ( size );
}

// Returns a pair of: < flattened index, newly inserted >.
std::pair<int32_t, bool> IndexFlattener::get_flattened_index ( int32_t position_index, int32_t texcoord_index, int32_t normal_index ) {
    if ( position_index >= static_cast<int32_t> ( table_.size() ) ) {
        table_.resize ( position_index + 1 );
    }
    // First, optimistically look up position_index in the table.
    IndexType& index = table_[position_index];
    if ( index.position_or_flat == K_INDEX_UNKNOWN ) {
        // This is the first time we've seen this position in the table,
        // so fill it. Since the table is indexed by position, we can
        // use the position_or_flat_index field to store the flat index.
        const int32_t flat_index = count_++;
        index.position_or_flat = flat_index;
        index.texcoord = texcoord_index;
        index.normal = normal_index;
        return std::make_pair ( flat_index, true );
    } else if ( index.position_or_flat == K_INDEX_NOT_IN_TABLE ) {
        // There are multiple flattened indices at this position index,
        // so resort to the map.
        return get_flattened_index_from_map ( position_index, texcoord_index, normal_index );
    } else if ( index.texcoord == texcoord_index &&
                index.normal == normal_index ) {
        // The other indices match, so we can use the value cached in
        // the table.
        return std::make_pair ( index.position_or_flat, false );
    }
    // The other indices don't match, so we mark this table entry,
    // and insert both the old and new indices into the map.
    const IndexType old_index ( position_index, index.texcoord, index.normal );
    map_.insert ( std::make_pair ( old_index, index.position_or_flat ) );
    index.position_or_flat = K_INDEX_NOT_IN_TABLE;
    const IndexType new_index ( position_index, texcoord_index, normal_index );
    const int32_t flat_index = count_++;
    map_.insert ( std::make_pair ( new_index, flat_index ) );
    return std::make_pair ( flat_index, true );
}

std::pair<int32_t, bool> IndexFlattener::get_flattened_index_from_map ( int32_t position_index, int32_t texcoord_index, int32_t normal_index ) {
    IndexType index ( position_index, texcoord_index, normal_index );
    MapType::iterator iter = map_.lower_bound ( index );
    if ( iter == map_.end() || iter->first != index ) {
        const int32_t flat_index = count_++;
        map_.insert ( iter, std::make_pair ( index, flat_index ) );
        return std::make_pair ( flat_index, true );
    } else {
        return std::make_pair ( iter->second, false );
    }
}

DrawBatch::DrawBatch()
    : flattener_ ( 0 ),
      current_group_line_ ( 0xFFFFFFFF ) {
}

const std::vector<GroupStart>& DrawBatch::group_starts() const {
    return group_starts_;
}

void DrawBatch::init ( AttribList* positions, AttribList* texcoords, AttribList* normals ) {
    positions_ = positions;
    texcoords_ = texcoords;
    normals_ = normals;
    flattener_.reserve ( 1024 );
}

void DrawBatch::add_triangle ( uint32_t group_line, int32_t* indices ) {
    if ( group_line != current_group_line_ ) {
        current_group_line_ = group_line;
        GroupStart group_start;
        group_start.offset = draw_mesh_.indices.size();
        group_start.group_line = group_line;
        group_start.min_index = std::numeric_limits<int>::max();
        group_start.max_index = std::numeric_limits<int>::min();
        group_start.bounds.clear();
        group_starts_.push_back ( group_start );
    }
    GroupStart& group = group_starts_.back();
    for ( size_t i = 0; i < 9; i += 3 ) {
        // .OBJ files use 1-based indexing.
        const int position_index = indices[i + 0] - 1;
        const int texcoord_index = indices[i + 1] - 1;
        const int normal_index = indices[i + 2] - 1;
        const std::pair<int, bool> flattened = flattener_.get_flattened_index (
                position_index, texcoord_index, normal_index );
        const int flat_index = flattened.first;
        assert ( flat_index >= 0 );
        draw_mesh_.indices.push_back ( flat_index );
        if ( flattened.second ) {
            // This is a new index. Keep track of index ranges and vertex
            // bounds.
            if ( flat_index > group.max_index ) {
                group.max_index = flat_index;
            }
            if ( flat_index < group.min_index ) {
                group.min_index = flat_index;
            }
            const size_t new_loc = draw_mesh_.attribs.size();
            assert ( 8*size_t ( flat_index ) == new_loc );
            for ( size_t i = 0; i < position_dim(); ++i ) {
                draw_mesh_.attribs.push_back (
                    positions_->at ( position_dim() * position_index + i ) );
            }
            if ( texcoord_index == -1 ) {
                for ( size_t i = 0; i < texcoord_dim(); ++i ) {
                    draw_mesh_.attribs.push_back ( 0 );
                }
            } else {
                for ( size_t i = 0; i < texcoord_dim(); ++i ) {
                    draw_mesh_.attribs.push_back (
                        texcoords_->at ( texcoord_dim() * texcoord_index + i ) );
                }
            }
            if ( normal_index == -1 ) {
                for ( size_t i = 0; i < normal_dim(); ++i ) {
                    draw_mesh_.attribs.push_back ( 0 );
                }
            } else {
                for ( size_t i = 0; i < normal_dim(); ++i ) {
                    draw_mesh_.attribs.push_back (
                        normals_->at ( normal_dim() * normal_index + i ) );
                }
            }
            // TODO: is the covariance body useful for anything?
            group.bounds.enclose_attrib ( &draw_mesh_.attribs[new_loc] );
        }
    }
}

const DrawMesh& DrawBatch::draw_mesh() const {
    return draw_mesh_;
}

void Material::dump_json ( FILE* out ) const {
    fprintf ( out, "    \"%s\": { ", name.c_str() );
    if ( map_kd.empty() ) {
        fprintf ( out, "\"kd\": [%hu, %hu, %hu] }",
                  quantize ( kd[0], 0, 1, 255 ),
                  quantize ( kd[1], 0, 1, 255 ),
                  quantize ( kd[2], 0, 1, 255 ) );
    } else {
        fprintf ( out, "\"map_kd\": \"%s\" }", map_kd.c_str() );
    }
}

WavefrontMtlFile::WavefrontMtlFile ( FILE* fp ) {
    parse_file ( fp );
}

const MaterialList& WavefrontMtlFile::materials() const {
    return materials_;
}

// TODO: factor this parsing stuff out.
void WavefrontMtlFile::parse_file ( FILE* fp ) {
    // TODO: don't use a fixed-size buffer.
    const size_t kLineBufferSize = 256;
    char buffer[kLineBufferSize];
    unsigned int line_num = 1;
    while ( fgets ( buffer, kLineBufferSize, fp ) != NULL ) {
        char* stripped = strip_leading_whitespace ( buffer );
        terminate_at_newline_or_comment ( stripped );
        parse_line ( stripped, line_num++ );
    }
}

void WavefrontMtlFile::parse_line ( const char* line, uint32_t line_num ) {
    switch ( *line ) {
    case 'K':
        parse_color ( line + 1, line_num );
        break;
    case 'm':
        if ( 0 == strncmp ( line + 1, "ap_Kd", 5 ) ) {
            parse_map_kd ( line + 6, line_num );
        }
        break;
    case 'n':
        if ( 0 == strncmp ( line + 1, "ewmtl", 5 ) ) {
            parse_new_mtl ( line + 6, line_num );
        }
    default:
        break;
    }
}

void WavefrontMtlFile::parse_color ( const char* line, unsigned int line_num ) {
    switch ( *line ) {
    case 'd': {
        ShortFloatList floats;
        floats.parse_line ( line + 1 );
        float* Kd = current_->kd;
        Kd[0] = floats[0];
        Kd[1] = floats[1];
        Kd[2] = floats[2];
        break;
    }
    default:
        break;
    }
}

void WavefrontMtlFile::parse_map_kd ( const char* line, uint32_t line_num ) {
    current_->map_kd = strip_leading_whitespace ( line );
}

void WavefrontMtlFile::parse_new_mtl ( const char* line, uint32_t line_num ) {
    materials_.push_back ( Material() );
    current_ = &materials_.back();
    to_lower ( strip_leading_whitespace ( line ), &current_->name );
}

WavefrontObjFile::WavefrontObjFile ( FILE* fp ) {
    current_batch_ = &material_batches_[""];
    current_batch_->init ( &positions_, &texcoords_, &normals_ );
    current_group_line_ = 0;
    line_to_groups_.insert ( std::make_pair ( 0, "default" ) );
    parse_file ( fp );
}

const MaterialList& WavefrontObjFile::materials() const {
    return materials_;
}

const MaterialBatches& WavefrontObjFile::material_batches() const {
    return material_batches_;
}

const std::string& WavefrontObjFile::line_to_group ( uint32_t line ) const {
    typedef LineToGroups::const_iterator Iterator;
    typedef std::pair<Iterator, Iterator> EqualRange;
    EqualRange equal_range = line_to_groups_.equal_range ( line );
    const std::string* best_group = NULL;
    int best_count = 0;
    for ( Iterator iter = equal_range.first; iter != equal_range.second;
            ++iter ) {
        const std::string& group = iter->second;
        const int count = group_counts_.find ( group )->second;
        if ( !best_group || ( count < best_count ) ) {
            best_group = &group;
            best_count = count;
        }
    }
    if ( !best_group ) {
        error_line ( "no suitable group found", line );
    }
    return *best_group;
}

void WavefrontObjFile::dump_debug() const {
    printf ( "positions size: " PRIuS "\n"
             "texcoords size: " PRIuS "\n"
             "normals size: " PRIuS "\n",
             positions_.size(), texcoords_.size(), normals_.size() );
}

void WavefrontObjFile::parse_file ( FILE* fp ) {
    // TODO: don't use a fixed-size buffer.
    const size_t kLineBufferSize = 256;
    char buffer[kLineBufferSize] = { 0 };
    unsigned int line_num = 1;
    while ( fgets ( buffer, kLineBufferSize, fp ) != NULL ) {
        char* stripped = strip_leading_whitespace ( buffer );
        terminate_at_newline_or_comment ( stripped );
        parse_line ( stripped, line_num++ );
    }
}

void WavefrontObjFile::parse_line ( const char* line, uint32_t line_num ) {
    switch ( *line ) {
    case 'v':
        parse_attrib ( line + 1, line_num );
        break;
    case 'f':
        parse_face ( line + 1, line_num );
        break;
    case 'g':
        if ( isspace ( line[1] ) ) {
            parse_group ( line + 2, line_num );
        } else {
            goto unknown;
        }
        break;
    case '\0':
    case '#':
        break;  // Do nothing for comments or blank lines.
    case 'p':
        warn_line ( "point unsupported", line_num );
        break;
    case 'l':
        warn_line ( "line unsupported", line_num );
        break;
    case 'u':
        if ( 0 == strncmp ( line + 1, "semtl", 5 ) ) {
            parse_use_mtl ( line + 6, line_num );
        } else {
            goto unknown;
        }
        break;
    case 'm':
        if ( 0 == strncmp ( line + 1, "tllib", 5 ) ) {
            parse_mtl_lib ( line + 6, line_num );
        } else {
            goto unknown;
        }
        break;
    case 's':
        parse_smoothing_group ( line + 1, line_num );
        break;
    unknown:
    default:
        warn_line ( "unknown keyword", line_num );
        break;
    }
}

void WavefrontObjFile::parse_attrib ( const char* line, uint32_t line_num ) {
    ShortFloatList floats;
    floats.parse_line ( line + 1 );
    if ( isspace ( *line ) ) {
        parse_position ( floats, line_num );
    } else if ( *line == 't' ) {
        parse_tex_coord ( floats, line_num );
    } else if ( *line == 'n' ) {
        parse_normal ( floats, line_num );
    } else {
        warn_line ( "unknown attribute format", line_num );
    }
}

void WavefrontObjFile::parse_position ( const ShortFloatList& floats, uint32_t line_num ) {
    if ( floats.size() != position_dim() &&
            floats.size() != 6 ) { // ignore r g b for now.
        error_line ( "bad position", line_num );
    }
    floats.append_n_to ( &positions_, position_dim() );
}

void WavefrontObjFile::parse_tex_coord ( const ShortFloatList& floats, unsigned int line_num ) {
    if ( ( floats.size() < 1 ) || ( floats.size() > 3 ) ) {
        // TODO: correctly handle 3-D texcoords intead of just
        // truncating.
        error_line ( "bad texcoord", line_num );
    }
    floats.append_n_to ( &texcoords_, texcoord_dim() );
}

void WavefrontObjFile::parse_normal ( const ShortFloatList& floats, uint32_t line_num ) {
    if ( floats.size() != normal_dim() ) {
        error_line ( "bad normal", line_num );
    }
    // Normalize to avoid out-of-bounds quantization. This should be
    // optional, in case someone wants to be using the normal magnitude as
    // something meaningful.
    const float x = floats[0];
    const float y = floats[1];
    const float z = floats[2];
    const float scale = 1.0/sqrt ( x*x + y*y + z*z );
    if ( std::isfinite ( scale ) ) {
        normals_.push_back ( scale * x );
        normals_.push_back ( scale * y );
        normals_.push_back ( scale * z );
    } else {
        normals_.push_back ( 0 );
        normals_.push_back ( 0 );
        normals_.push_back ( 0 );
    }
}

// Parses faces and converts to triangle fans.
// This is not a particularly good tesselation in general case, but it is really simple, and is perfectly fine for triangles and quads.
void WavefrontObjFile::parse_face ( const char* line, uint32_t line_num ) {
    // Also handle face outlines as faces.
    if ( *line == 'o' ) ++line;

    // TODO: instead of storing these indices as-is, it might make
    // sense to flatten them right away. This can reduce memory
    // consumption and improve access locality, especially since .OBJ
    // face indices are so needlessly large.
    int indices[9] = { 0 };
    // The first index acts as the pivot for the triangle fan.
    line = parse_indices ( line, line_num, indices + 0, indices + 1, indices + 2 );
    if ( line == NULL ) {
        error_line ( "bad first index", line_num );
    }
    line = parse_indices ( line, line_num, indices + 3, indices + 4, indices + 5 );
    if ( line == NULL ) {
        error_line ( "bad second index", line_num );
    }
    // After the first two indices, each index introduces a new
    // triangle to the fan.
    while ( ( line = parse_indices ( line, line_num,
                                    indices + 6, indices + 7, indices + 8 ) ) ) {
        current_batch_->add_triangle ( current_group_line_, indices );
        // The most recent vertex is reused for the next triangle.
        indices[3] = indices[6];
        indices[4] = indices[7];
        indices[5] = indices[8];
        indices[6] = indices[7] = indices[8] = 0;
    }
}

// Parse a single group of indices, separated by slashes ('/').
// TODO: convert negative indices (that is, relative to the end of the current vertex positions) to more conventional positive indices.
const char* WavefrontObjFile::parse_indices ( const char* line, uint32_t line_num, int32_t* position_index, int32_t* texcoord_index, int32_t* normal_index ) {
    const char* endptr = NULL;
    *position_index = str_to_int ( line, &endptr );
    if ( *position_index == 0 ) {
        return NULL;
    }
    if ( endptr != NULL && *endptr == '/' ) {
        *texcoord_index = str_to_int ( endptr + 1, &endptr );
    } else {
        *texcoord_index = *normal_index = 0;
    }
    if ( endptr != NULL && *endptr == '/' ) {
        *normal_index = str_to_int ( endptr + 1, &endptr );
    } else {
        *normal_index = 0;
    }
    return endptr;
}

// .OBJ files can specify multiple groups for a set of faces.
// This implementation finds the "most unique" group for a set of faces and uses that for the batch.
// In the first pass, we use the line number of the "g" command to tag the faces.
// Afterwards, after we collect group populations, we can go back and give them real names.
void WavefrontObjFile::parse_group ( const char* line, uint32_t line_num ) {
    std::string token;
    while ( ( line = consume_first_token ( line, &token ) ) ) {
        to_lower_inplace ( &token );
        group_counts_[token]++;
        line_to_groups_.insert ( std::make_pair ( line_num, token ) );
    }
    current_group_line_ = line_num;
}

void WavefrontObjFile::parse_smoothing_group ( const char* line, uint32_t line_num ) {
    static bool once = true;
    if ( once ) {
        warn_line ( "s ignored", line_num );
        once = false;
    }
}

void WavefrontObjFile::parse_mtl_lib ( const char* line, uint32_t line_num ) {
    FILE* fp = fopen ( strip_leading_whitespace ( line ), "r" );
    if ( !fp ) {
        warn_line ( "mtllib not found", line_num );
        return;
    }
    WavefrontMtlFile mtlfile ( fp );
    fclose ( fp );
    materials_ = mtlfile.materials();
    for ( size_t i = 0; i < materials_.size(); ++i ) {
        DrawBatch& draw_batch = material_batches_[materials_[i].name];
        draw_batch.init ( &positions_, &texcoords_, &normals_ );
    }
}

void WavefrontObjFile::parse_use_mtl ( const char* line, uint32_t line_num ) {
    std::string usemtl;
    to_lower ( strip_leading_whitespace ( line ), &usemtl );
    MaterialBatches::iterator iter = material_batches_.find ( usemtl );
    if ( iter == material_batches_.end() ) {
        error_line ( "material not found", line_num );
    }
    current_batch_ = &iter->second;
}

void WavefrontObjFile::warn_line ( const char* why, uint32_t line_num ) const {
    fprintf ( stderr, "WARNING: %s at line %u\n", why, line_num );
}

void WavefrontObjFile::error_line ( const char* why, unsigned int line_num ) const {
    fprintf ( stderr, "ERROR: %s at line %u\n", why, line_num );
    exit ( -1 );
}

} // namespace webgl_loader
