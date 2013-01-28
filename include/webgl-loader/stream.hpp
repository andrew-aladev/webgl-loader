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

#ifndef WEBGL_LOADER_STREAM_HPP
#define WEBGL_LOADER_STREAM_HPP

#include <webgl-loader/base.hpp>

namespace webgl_loader {

// An abstract interface to allow appending bytes to various streams.
class ByteSinkInterface {
public:
    virtual void put ( char c ) = 0;
    virtual size_t put_n ( const char* data, size_t len ) = 0;
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
    NullSink();
    virtual void put ( char );
    virtual size_t put_n ( const char*, size_t len );
};

class FileSink : public ByteSinkInterface {
private:
    FILE *fp_;  // unowned.
public:
    explicit FileSink ( FILE* fp );
    virtual void put ( char c );

    virtual size_t put_n ( const char* data, size_t len );
};

class VectorSink : public ByteSinkInterface {
private:
    std::vector<char>* vec_;  // unowned.

public:
    explicit VectorSink ( std::vector<char>* vec );
    virtual void put ( char c );
    virtual size_t put_n ( const char* data, size_t len );
};

class StringSink : public ByteSinkInterface {
private:
    std::string* str_;  // unowned.

public:
    explicit StringSink ( std::string* str );
    virtual void put ( char c );
    virtual size_t put_n ( const char* data, size_t len );
};

class ByteHistogramSink : public ByteSinkInterface {
private:
    size_t histo_[256];
    ByteSinkInterface* sink_;  // unowned.

public:
    explicit ByteHistogramSink ( ByteSinkInterface* sink );
    virtual void put ( char c );
    virtual size_t put_n ( const char* data, size_t len );
    const size_t* histo() const;
};

// TODO: does it make sense to have a global enum? How should
// new BufferedInput implementations define new error codes?
enum ErrorCode {
    K_NO_ERROR    = 0,
    K_END_OF_FILE = 1,
    K_FILE_ERROR  = 2,  // TODO: translate errno.
};

// Adapted from ryg's BufferedStream abstraction:
// http://fgiesen.wordpress.com/2011/11/21/buffer-centric-io/
class BufferedInput {
private:
    // Disallow copy and assign.
    BufferedInput ( const BufferedInput& );
    void operator= ( const BufferedInput& );

public:
    typedef ErrorCode ( *Refiller ) ( BufferedInput* );

protected:
    const char* begin_;
    const char* end_;
    Refiller refiller_;
    ErrorCode error_;

    static ErrorCode refill_zeroes ( BufferedInput* bi );
    static ErrorCode refill_end_of_file ( BufferedInput* bi );
    ErrorCode fail ( ErrorCode why );

public:
    BufferedInput ( Refiller refiller = refill_zeroes );
    BufferedInput ( const char* data, size_t length );

    const char* begin() const;
    const char* end() const;
    const char* cursor;

    ErrorCode error() const;
    ErrorCode refill();
};

class BufferedInputStream : public BufferedInput {
private:
    FILE* fp_;
    char* buf_;
    size_t size_;

public:
    BufferedInputStream ( FILE* fp, char* buf, size_t size );

protected:
    static ErrorCode refill_fread ( BufferedInput* bi );

private:
    ErrorCode do_refill_fread();
};

}  // namespace webgl_loader

#endif  // WEBGL_LOADER_STREAM_HPP
