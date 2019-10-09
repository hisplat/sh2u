#pragma once

// #include "archive.h"

#include <iostream>
#include "serialize/archive.h"

namespace tools {

class Buffer 
{
public:
    Buffer(const char * buffer, int size);
    Buffer(int size, char c = '\0');
    Buffer();
    Buffer(const Buffer& o);
    ~Buffer();

    int length() const { return len; }
    const char * buffer() const { return data; }
    bool isEmpty();

    void assign(const char * data, int len);
    void assign(int size, char c = '\0');

    void append(const char* data, int size);
    void erase_before(int offset);

    char& at(int offset);
    char& operator[](int offset);
    char operator[] (int offset) const;
    operator char*() { return data; }
    operator const char*() { return data; }
    operator unsigned char*() { return (unsigned char*)data; }
    operator void*() { return data; }
    Buffer& operator=(const Buffer& o);
    Buffer& operator=(const Buffer* o);
    unsigned char* operator+(int offset) { return (unsigned char*)data + offset; }

    int     mArgN;
    long    mArgL;
    void*   mArgP;

private:
    int     len;
    char*   data;
};

} // namespace tools

// 序列化:
// example:
//      tools::Buffer   buf;
//      tools::Archive  ar;
//      ar << buf;
Archive& operator<<(Archive& ar, tools::Buffer& buf);
Archive& operator>>(Archive& ar, tools::Buffer& buf);

// logging输出:
// example: 
//      tools::Buffer    buf;
//      ILOG() << buf;
std::ostream& operator<<(std::ostream& o, tools::Buffer& ub);









