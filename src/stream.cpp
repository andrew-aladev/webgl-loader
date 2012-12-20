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

#include <webgl-loader/utf8.hpp>

namespace webgl_loader {

NullSink::NullSink() { }

void NullSink::put ( char ) { }

size_t NullSink::put_n ( const char*, size_t len ) {
    return len;
}

// |fp| is unowned and must not be NULL.
FileSink::FileSink ( FILE* fp )
    : fp_ ( fp ) {
}

void FileSink::put ( char c ) {
    putc ( c, fp_ );
}

size_t FileSink::put_n ( const char* data, size_t len ) {
    return fwrite ( data, 1, len, fp_ );
}

// |vec| is unowned and must not be NULL.
VectorSink::VectorSink ( std::vector<char>* vec )
    : vec_ ( vec ) {
}

void VectorSink::put ( char c ) {
    vec_->push_back ( c );
}

size_t VectorSink::put_n ( const char* data, size_t len ) {
    vec_->insert ( vec_->end(), data, data + len );
    return len;
}

// |str| is unowned and must not be NULL.
StringSink::StringSink ( std::string* str )
    : str_ ( str ) {
    assert ( str != NULL );
}

void StringSink::put ( char c ) {
    str_->push_back ( c );
}

size_t StringSink::put_n ( const char* data, size_t len ) {
    str_->append ( data, len );
    return len;
}

// |sink| in unowned and must not be NULL.
ByteHistogramSink::ByteHistogramSink ( ByteSinkInterface* sink )
    : sink_ ( sink ) {
    memset ( histo_, 0, sizeof ( histo_ ) );
}

void ByteHistogramSink::put ( char c ) {
    histo_[static_cast<uint8_t> ( c )]++;
    sink_->put ( c );
}

size_t ByteHistogramSink::put_n ( const char* data, size_t len ) {
    const char* const end = data + len;
    for ( const char* iter = data; iter != end; ++iter ) {
        histo_[static_cast<uint8_t> ( *iter )]++;
    }
    return sink_->put_n ( data, len );
}

const size_t* ByteHistogramSink::histo() const {
    return histo_;
}

BufferedInput::BufferedInput ( Refiller refiller)
    : cursor ( NULL ),
      begin_ ( NULL ),
      end_ ( NULL ),
      refiller_ ( refiller ),
      error_ ( K_NO_ERROR ) {
}

// InitFromMemory.
BufferedInput::BufferedInput ( const char* data, size_t length )
    : cursor ( data ),
      begin_ ( data ),
      end_ ( data + length ),
      refiller_ ( refill_end_of_file ),
      error_ ( K_NO_ERROR ) {
}

const char* BufferedInput::begin() const {
    return begin_;
}

const char* BufferedInput::end() const {
    return end_;
}

const char* cursor;

ErrorCode BufferedInput::error() const {
    assert ( begin() <= cursor );
    assert ( cursor <= end() );
    return error_;
}

ErrorCode BufferedInput::refill() {
    assert ( begin() <= cursor );
    assert ( cursor <= end() );
    if ( cursor == end() ) {
        error_ = refiller_ ( this );
    }
    return error_;
}

ErrorCode BufferedInput::refill_zeroes ( BufferedInput* bi ) {
    static const char kZeroes[64] = { 0 };
    bi->cursor = kZeroes;
    bi->begin_ = kZeroes;
    bi->end_ = kZeroes + sizeof ( kZeroes );
    return bi->error_;
}

ErrorCode BufferedInput::refill_end_of_file ( BufferedInput* bi ) {
    return bi->fail ( K_END_OF_FILE );
}

ErrorCode BufferedInput::fail ( ErrorCode why ) {
    error_ = why;
    refiller_ = refill_zeroes;
    return refill();
}

BufferedInputStream::BufferedInputStream ( FILE* fp, char* buf, size_t size )
    : BufferedInput ( refill_fread ),
      fp_ ( fp ),
      buf_ ( buf ),
      size_ ( size ) {
    assert ( buf != NULL );
    // Disable buffering since we're doing it ourselves.
    // TODO check error.
    setvbuf ( fp_, NULL, _IONBF, 0 );
    cursor = buf;
    begin_ = buf;
    end_ = buf;
}

// TODO: figure out how to automate this casting pattern.
ErrorCode BufferedInputStream::refill_fread ( BufferedInput* bi ) {
    return static_cast<BufferedInputStream*> ( bi )->do_refill_fread();
}

ErrorCode BufferedInputStream::do_refill_fread() {
    const size_t bytes_read = fread ( buf_, 1, size_, fp_ );
    cursor = begin_;
    end_ = begin_ + bytes_read;
    if ( bytes_read < size_ ) {
        if ( feof ( fp_ ) ) {
            refiller_ = refill_end_of_file;
        } else if ( ferror ( fp_ ) ) {
            return fail ( K_FILE_ERROR );
        }
    }
    return K_NO_ERROR;
}

}  // namespace webgl_loader
