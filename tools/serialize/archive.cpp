

#include "archive.h"
#include "serialization.h"

#include "../dump.h"

Archive::Archive()
{
}

Archive::~Archive()
{
    reset();
}

Archive::Archive(const Archive& o)
{
}

Archive& Archive::operator=(const Archive& o)
{
    return *this;
}

Archive& Archive::operator<<(std::string str) {
    Data* data = new Data(str.length() + 1, str.c_str());
    mDataList.push_back(data);
    return *this;
}

Archive& Archive::operator>>(std::string& str) {
    if (mDataList.empty())
        return *this;
    Data* data = mDataList.front();
    mDataList.pop_front();
    str = std::string((char *)data->getData());
    delete data;
    return *this;
}
    
Archive& Archive::operator<<(Serialization& s)
{
    s.Serialize(*this);
    return *this;
}

Archive& Archive::operator>>(Serialization& s)
{
    s.Deserialize(*this);
    return *this;
}

void Archive::Save(const char * file)
{
    FILE * fp = fopen(file, "wb+");
    if (!fp) {
        return;
    }
    std::list<Data*>::const_iterator it;
    for (it = mDataList.begin(); it != mDataList.end(); it++) {
        (*it)->Save(fp);
    }
    fclose(fp);
}

bool Archive::Load(const char * file)
{
    reset();
    FILE * fp = fopen(file, "rb");
    if (!fp) {
        return false;
    }
    while (!feof(fp)) {
        Data* data = new Data(fp);
        mDataList.push_back(data);
    }
    fclose(fp);
    return true;
}

bool Archive::fromBuffer(const char * buffer, int len)
{
    reset();
    int count = 0;
    while (count < len) {
        Data* data = new Data(buffer + count);
        mDataList.push_back(data);

        /*
        char    buffer[1024];
        int ret = data->dump(buffer, sizeof(buffer));
        printf("data:\n");
        tools::dump(buffer, ret);
        */

        if (data->getLength() <= 0) {
            reset();
            return false;
        }
        count += data->getSize();
    }
    return true;
}

int Archive::toBuffer(char* buffer, int len)
{
    int count = 0;
    std::list<Data*>::const_iterator it;
    for (it = mDataList.begin(); it != mDataList.end() && count < len; it++) {
        int ret = (*it)->dumpBuffer(buffer + count, len - count);
        count += ret;
    }
    return count;
}


void Archive::reset()
{
    if (mDataList.empty())
        return;
    std::list<Data*>::const_iterator it;
    for (it = mDataList.begin(); it != mDataList.end(); it++) {
        delete *it;
    }
    mDataList.clear();
}

Archive::Data::Data(int l, const void * d)
    : data(NULL)
{
    assign(l, d);
}

Archive::Data::Data(const void* d)
    : len(0)
    , data(NULL)
{
    memcpy(&len, d, sizeof(len));
    // printf("len = %d\n", len);
    if (len > 0) {
        assign(len, (char *)d + sizeof(len));
    }
}

Archive::Data::~Data()
{
    assign(0, NULL);
}

Archive::Data::Data(const Data& o)
    : data(NULL)
{
    assign(o.len, o.data);
}

Archive::Data::Data(FILE* fp)
    : len(0)
    , data(NULL)
{
    int ret = (int)fread(&len, 1, sizeof(len), fp);
    if (ret < 0) {
        len = 0;
        return;
    }
    if (len > 0) {
        data = new char[len];
        ret = (int)fread(data, 1, len, fp);
        if (ret < 0) {
            len = 0;
            data = NULL;
            return;
        }
    }
}

Archive::Data& Archive::Data::operator=(const Data& o)
{
    assign(o.len, o.data);
    return *this;
}

void Archive::Data::assign(int l, const void * d)
{
    if (data)
        delete [] data;
    len = 0;
    data = NULL;
    if (l <= 0)
        return;

    len = l;
    data = new char[len];
    memcpy(data, d, len);
}

int Archive::Data::dump(void* p, int l)
{
    if (l > len)
        l = len;
    memcpy(p, data, l);
    return l;
}

int Archive::Data::dumpBuffer(void* p, int l)
{
    int ll = l;
    if (ll > (int)sizeof(len)) {
        ll = (int)sizeof(len);
    }
    memcpy(p, &len, ll);

    int lll = l - ll;
    if (lll > len)
        lll = len;

    memcpy((char *)p + ll, data, lll);
    return ll + lll;
}

void Archive::Data::Save(FILE* fp)
{
    fwrite(&len, 1, sizeof(len), fp);
    fwrite(data, 1, len, fp);
}




