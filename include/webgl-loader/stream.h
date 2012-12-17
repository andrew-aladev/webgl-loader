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

#ifndef WEBGL_LOADER_STREAM_H_
#define WEBGL_LOADER_STREAM_H_

#include <stdio.h>
#include <string>
#include <vector>

#include <webgl-loader/base.h>

namespace webgl_loader {

// An abstract interface to allow appending bytes to various streams.
class ByteSinkInterface {
public:
    virtual void Put ( char c ) = 0;
    virtual size_t PutN ( const char* data, size_t len ) = 0;
    virtual ~ByteSinkInterface() { }

protected:
    ByteSinkInterface() { }

private:
    // Disallow copy and assignment.
    ByteSinkInterface ( const ByteSinkInterface& );
    void operator= ( const ByteSinkInterface& );
};

// None of the concrete implementations actually own the backing data.
// They should be safe to copy.

class NullSink : public ByteSinkInterface {
public:
    NullSink() { }

    virtual void Put ( char ) { }

    virtual size_t PutN ( const char*, size_t len ) {
        return len;
    }
};

class FileSink : public ByteSinkInterface {
public:
    // |fp| is unowned and must not be NULL.
    explicit FileSink ( FILE* fp )
        : fp_ ( fp ) {
    }

    virtual void Put ( char c ) {
        PutChar ( c, fp_ );
    }

    virtual size_t PutN ( const char* data, size_t len ) {
        return fwrite ( data, 1, len, fp_ );
    }

private:
    FILE *fp_;  // unowned.
};

class VectorSink : public ByteSinkInterface {
public:
    // |vec| is unowned and must not be NULL.
    explicit VectorSink ( std::vector<char>* vec )
        : vec_ ( vec ) {
    }

    virtual void Put ( char c ) {
        vec_->push_back ( c );
    }

    virtual size_t PutN ( const char* data, size_t len ) {
        vec_->insert ( vec_->end(), data, data + len );
        return len;
    }

private:
    std::vector<char>* vec_;  // unowned.
};

class StringSink : public ByteSinkInterface {
public:
    // |str| is unowned and must not be NULL.
    explicit StringSink ( std::string* str )
        : str_ ( str ) {
        assert ( str != NULL );
    }

    virtual void Put ( char c ) {
        str_->push_back ( c );
    }

    virtual size_t PutN ( const char* data, size_t len ) {
        str_->append ( data, len );
        return len;
    }

private:
    std::string* str_;  // unowned.
};

class ByteHistogramSink : public ByteSinkInterface {
public:
    // |sink| in unowned and must not be NULL.
    explicit ByteHistogramSink ( ByteSinkInterface* sink )
        : sink_ ( sink ) {
        memset ( histo_, 0, sizeof ( histo_ ) );
    }

    virtual void Put ( char c ) {
        histo_[static_cast<uint8_t> ( c )]++;
        sink_->Put ( c );
    }

    virtual size_t PutN ( const char* data, size_t len ) {
        const char* const end = data + len;
        for ( const char* iter = data; iter != end; ++iter ) {
            histo_[static_cast<uint8_t> ( *iter )]++;
        }
        return sink_->PutN ( data, len );
    }

    const size_t* histo() const {
        return histo_;
    }

private:
    size_t histo_[256];
    ByteSinkInterface* sink_;  // unowned.
};

// TODO: does it make sense to have a global enum? How should
// new BufferedInput implementations define new error codes?
enum ErrorCode {
    kNoError = 0,
    kEndOfFile = 1,
    kFileError = 2,  // TODO: translate errno.
};

// Adapted from ryg's BufferedStream abstraction:
// http://fgiesen.wordpress.com/2011/11/21/buffer-centric-io/
class BufferedInput {
public:
    typedef ErrorCode ( *Refiller ) ( BufferedInput* );

    BufferedInput ( Refiller refiller = RefillZeroes )
        : cursor ( NULL ),
          begin_ ( NULL ),
          end_ ( NULL ),
          refiller_ ( refiller ),
          error_ ( kNoError ) {
    }

    // InitFromMemory.
    BufferedInput ( const char* data, size_t length )
        : cursor ( data ),
          begin_ ( data ),
          end_ ( data + length ),
          refiller_ ( RefillEndOfFile ),
          error_ ( kNoError ) {
    }

    const char* begin() const {
        return begin_;
    }

    const char* end() const {
        return end_;
    }

    const char* cursor;

    ErrorCode error() const {
        assert ( begin() <= cursor );
        assert ( cursor <= end() );
        return error_;
    }

    ErrorCode Refill() {
        assert ( begin() <= cursor );
        assert ( cursor <= end() );
        if ( cursor == end() ) {
            error_ = refiller_ ( this );
        }
        return error_;
    }

protected:
    static ErrorCode RefillZeroes ( BufferedInput* bi ) {
        static const char kZeroes[64] = { 0 };
        bi->cursor = kZeroes;
        bi->begin_ = kZeroes;
        bi->end_ = kZeroes + sizeof ( kZeroes );
        return bi->error_;
    }

    static ErrorCode RefillEndOfFile ( BufferedInput* bi ) {
        return bi->fail ( kEndOfFile );
    }

    ErrorCode fail ( ErrorCode why ) {
        error_ = why;
        refiller_ = RefillZeroes;
        return Refill();
    }

    const char* begin_;
    const char* end_;
    Refiller refiller_;
    ErrorCode error_;

private:
    // Disallow copy and assign.
    BufferedInput ( const BufferedInput& );
    void operator= ( const BufferedInput& );
};

class BufferedInputStream : public BufferedInput {
public:
    BufferedInputStream ( FILE* fp, char* buf, size_t size )
        : BufferedInput ( RefillFread ),
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
protected:
    // TODO: figure out how to automate this casting pattern.
    static ErrorCode RefillFread ( BufferedInput* bi ) {
        return static_cast<BufferedInputStream*> ( bi )->DoRefillFread();
    }
private:
    ErrorCode DoRefillFread() {
        const size_t bytes_read = fread ( buf_, 1, size_, fp_ );
        cursor = begin_;
        end_ = begin_ + bytes_read;
        if ( bytes_read < size_ ) {
            if ( feof ( fp_ ) ) {
                refiller_ = RefillEndOfFile;
            } else if ( ferror ( fp_ ) ) {
                return fail ( kFileError );
            }
        }
        return kNoError;
    }

    FILE* fp_;
    char* buf_;
    size_t size_;
};

}  // namespace webgl_loader

#endif  // WEBGL_LOADER_STREAM_H_
