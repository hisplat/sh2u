

#include "buffer.h"
#include "tools/dump.h"
#include <stdlib.h>
#include <string.h>
#include <iomanip>

namespace tools {
Buffer::Buffer()
    : len(0)
    , data(NULL)
{
}

Buffer::Buffer(const char * buffer, int size)
    : len(0)
    , data(NULL)
{
    assign(buffer, size);
}

Buffer::Buffer(int size, char c)
    : len(0)
    , data(NULL)
{
    assign(size, c);
}

Buffer::Buffer(const Buffer& o)
    : len(0)
    , data(NULL)
{
    assign(o.data, o.len);
    mArgN = o.mArgN;
    mArgL = o.mArgL;
    mArgP = o.mArgP;
}

Buffer::~Buffer()
{
    assign((const char *)NULL, 0);
}

Buffer& Buffer::operator=(const Buffer& o)
{
    assign(o.data, o.len);
    mArgN = o.mArgN;
    mArgL = o.mArgL;
    mArgP = o.mArgP;
    return *this;
}

Buffer& Buffer::operator=(const Buffer* o)
{
    assign(o->data, o->len);
    mArgN = o->mArgN;
    mArgL = o->mArgL;
    mArgP = o->mArgP;
    return *this;
}

void Buffer::assign(const char * buffer, int size)
{
    if (data != NULL) {
        delete[] data;
    }
    len = size;
    data = NULL;

    if (len <= 0) {
        return;
    }
    data = new char[size];
    memcpy(data, buffer, size);
}

void Buffer::assign(int size, char c)
{
    if (data != NULL) {
        delete[] data;
    }
    len = size;
    data = NULL;

    if (size <= 0)
        return;

    data = new char[size];
    memset(data, c, size);
}

void Buffer::append(const char* buffer, int size)
{
    if (buffer == NULL || size <= 0)
        return;

    char * temp = new char[len + size];
    if (len > 0)
        memcpy(temp, data, len);
    memcpy(temp + len, buffer, size);

    if (data != NULL) {
        delete[] data;
    }
    data = temp;
    len = len + size;
}

void Buffer::erase_before(int offset)
{
    if (offset <= 0)
        return;
    if (offset >= len) {
        assign(0);
    } else {
        memmove(data, data + offset, len - offset);
        len -= offset;
    }
}

bool Buffer::isEmpty()
{
    return (data == NULL);
}

char& Buffer::at(int offset)
{
    if (offset >= len || offset < -len) {
        return *(char*)NULL;
    }
    if (offset < 0) {
        return data[len + offset];
    }
    return data[offset];
}

char& Buffer::operator[](int offset)
{
    return at(offset);
}

char Buffer::operator[] (int offset) const
{
    if (offset >= len || offset < -len) {
        return *(char*)NULL;
    }
    if (offset < 0) {
        return data[len + offset];
    }
    return data[offset];
}


} // namespace tools

Archive& operator<<(Archive& ar, tools::Buffer& buf)
{
    Archive::Data* data = new Archive::Data(buf.length(), buf.buffer());
    ar.mDataList.push_back(data);
    return ar;
}

Archive& operator>>(Archive& ar, tools::Buffer& buf)
{
    if (ar.mDataList.empty())
        return ar;
    Archive::Data* data = ar.mDataList.front();
    ar.mDataList.pop_front();
    buf.assign((const char *)data->getData(), data->getLength());
    delete data;
    return ar;
}

std::ostream& operator<<(std::ostream& o, tools::Buffer& ub)
{
    o << "Buffer(" << ub.length() << " bytes): {";
    int count = 0;
    int i;
    for (i = 0; i < ub.length(); i++) {
        char p = ub.buffer()[i];
        if ((p >= 36 && p <= 126) || p == '"' || p == ' ') {
            o << p;
            count++;
        } else {
            o << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (p & 0xff);
        }
    }
    o << "}";

    if (count > ub.length() / 2) {
        char    temp[1024] = {0};
        memcpy(temp, ub.buffer(), ub.length() < 1023 ? ub.length() : 1023);
        o << " {" << temp << "}";
    } else {
        // tools::dump(ub.buffer(), ub.length());
    }
    return o;
}





