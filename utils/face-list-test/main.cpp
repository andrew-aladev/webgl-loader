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

#include <cstdio>

#include <algorithm>

#include <webgl-loader/base.hpp>

class FaceList {
public:
    static const size_t kInlinedCapacity = 6;

    FaceList()
        : faces_ ( capacity_or_buffer_.buffer ),
          size_ ( 0 ) {
    }

    FaceList ( const FaceList& that )
        : faces_ ( capacity_or_buffer_.buffer ),
          size_ ( that.size_ ) {
        if ( !IsInlined() ) {
            faces_ = Malloc ( size_ );
        }
        Memcpy ( faces_, that.faces_, size_ );
    }

    ~FaceList() {
        clear();
    }

    size_t size() const {
        return size_;
    }

    size_t capacity() const {
        return ( size_ <= kInlinedCapacity ) ?
               kInlinedCapacity : capacity_or_buffer_.capacity;
    }

    int& operator[] ( size_t a ) {
        return faces_[a];
    }

    int operator[] ( size_t a ) const {
        return faces_[a];
    }

    typedef int* iterator;
    typedef const int* const_iterator;

    iterator begin() {
        return faces_;
    }
    const_iterator begin() const {
        return faces_;
    }

    iterator end() {
        return faces_ + size_;
    }
    const_iterator end() const {
        return faces_ + size_;
    }

    void clear() {
        if ( !IsInlined() ) {
            free ( faces_ );
            faces_ = capacity_or_buffer_.buffer;
        }
        size_ = 0;
    }

    int& back() {
        return faces_[size_ - 1];
    }

    int back() const {
        return faces_[size_ - 1];
    }

    void push_back ( int i ) {
        if ( size_ == kInlinedCapacity ) {
            const size_t kMinAlloc = 16;
            faces_ = Malloc ( kMinAlloc );
            Memcpy ( faces_, capacity_or_buffer_.buffer, size_ );
            capacity_or_buffer_.capacity = kMinAlloc;
        } else if ( !IsInlined() && ( size_ == capacity_or_buffer_.capacity ) ) {
            faces_ = Realloc ( faces_, 2*capacity_or_buffer_.capacity );
        }
        faces_[size_++] = i;
    }

    void pop_back() {
        --size_;
        if ( size_ == kInlinedCapacity ) {
            Memcpy ( capacity_or_buffer_.buffer, faces_, kInlinedCapacity );
            free ( faces_ );
            faces_ = capacity_or_buffer_.buffer;
        }
    }

private:
    bool IsInlined() const {
        return size_ <= kInlinedCapacity;
    }

    static void Memcpy ( int *to, const int* from, size_t n ) {
        memcpy ( to, from, sizeof ( int ) *n );
    }

    static int* Malloc ( size_t sz ) {
        return static_cast<int*> ( malloc ( sizeof ( int ) *sz ) );
    }

    static int* Realloc ( int *ptr, size_t sz ) {
        return static_cast<int*> ( realloc ( ptr, sizeof ( int ) *sz ) );
    }

    int* faces_;
    size_t size_;
    union {
        size_t capacity;
        int buffer[kInlinedCapacity];
    } capacity_or_buffer_;
};

void TestBasic() {
    FaceList face_list;
    assert ( 0 == face_list.size() );
    face_list.push_back ( 10 );
    assert ( 1 == face_list.size() );
    assert ( 10 == face_list.back() );
    face_list.push_back ( 20 );
    assert ( 2 == face_list.size() );
    assert ( 20 == face_list.back() );
    face_list.back() = 30;
    assert ( 2 == face_list.size() );
    assert ( 30 == face_list.back() );
    face_list.clear();
    assert ( 0 == face_list.size() );
}

void TestPushClear ( size_t n ) {
    FaceList face_list;
    std::vector<int> v;
    for ( size_t i = 0; i < n; ++i ) {
        const int to_push = i * i;
        face_list.push_back ( to_push );
        v.push_back ( to_push );
        assert ( v.size() == face_list.size() );
        assert ( to_push == face_list.back() );
    }
    assert ( n == face_list.size() );
    assert ( std::equal ( face_list.begin(), face_list.end(), v.begin() ) );
    FaceList copied ( face_list );
    assert ( n == copied.size() );
    assert ( std::equal ( face_list.begin(), face_list.end(), copied.begin() ) );
    face_list.clear();
    assert ( 0 == face_list.size() );
}

void TestPushPopBack ( size_t n ) {
    FaceList face_list;
    std::vector<int> v;
    for ( size_t i = 0; i < n; ++i ) {
        const int to_push = i * i;
        face_list.push_back ( to_push );
        v.push_back ( to_push );
        assert ( v.size() == face_list.size() );
        assert ( to_push == face_list.back() );
    }
    for ( size_t i = n - 1; i > 0; --i ) {
        const int to_pop = i * i;
        assert ( v.size() == face_list.size() );
        assert ( to_pop == face_list.back() );
        face_list.pop_back();
        v.pop_back();
    }
    assert ( 0 == face_list.back() );
    assert ( 1 == face_list.size() );
    face_list.pop_back();
    assert ( 0 == face_list.size() );
}

int main ( int argc, char* argv[] ) {
    TestBasic();
    // Chose sizes to guarante both inlined->extern transition and
    // realloc.
    for ( size_t i = 4; i < 20; ++i ) {
        TestPushClear ( i );
        TestPushPopBack ( i );
    }
    return 0;
}
