

#pragma once

#include <sstream>
#include <string>
#include <list>
#include <string.h>
#include <stdio.h>

class Serialization;
class Archive
{
public:
    Archive();
    ~Archive();

    template <class T>
        Archive& operator<<(T t) {
            Data* data = new Data(sizeof(T), &t);
            mDataList.push_back(data);
            return *this;
        }

    template <class T>
        Archive& operator>>(T& t) {
            if (mDataList.empty())
                return *this;
            Data* data = mDataList.front();
            mDataList.pop_front();
            data->dump(&t, sizeof(T));
            delete data;
            return *this;
        }

    Archive& operator<<(std::string str);
    Archive& operator>>(std::string& str);
    Archive& operator<<(Serialization& s);
    Archive& operator>>(Serialization& s);

    void Save(const char * file);
    bool Load(const char * file);
    bool fromBuffer(const char * buffer, int len);
    int toBuffer(char* buffer, int len);

public:
    typedef enum {
        T_UNKNOWN,
        T_SCHAR,
        T_UCHAR,
        T_SHORT,
        T_USHORT,
        T_INT,
        T_UINT,
        T_LONG,
        T_ULONG,
        T_FLOAT,
        T_DOUBLE,
        T_STR,
    } TYPE;

    template <class T> TYPE gettype(T t) { return T_UNKNOWN; }
    TYPE gettype(signed char& t) { return T_SCHAR; }
    TYPE gettype(unsigned char& t) { return T_UCHAR; }
    TYPE gettype(short int& t) { return T_SHORT; }
    TYPE gettype(unsigned short int& t) { return T_USHORT; }
    TYPE gettype(int& t) { return T_INT; }
    TYPE gettype(unsigned int& t) { return T_UINT; }
    TYPE gettype(long& t) { return T_LONG; }
    TYPE gettype(unsigned long& t) { return T_ULONG; }
    TYPE gettype(float& t) { return T_FLOAT; }
    TYPE gettype(double& t) { return T_DOUBLE; }
    TYPE gettype(std::string& t) { return T_STR; }

private:
    Archive(const Archive& o);
    Archive& operator=(const Archive& o);

    void reset();

public:
    class Data {
    public:
        Data(int l, const void * d);
        Data(const Data& o);
        Data(FILE* fp);
        Data(const void* d);
        ~Data();

        Data& operator=(const Data& o);

        void assign(int l, const void * d);
        int dump(void* p, int l);
        int dumpBuffer(void* p, int l);
        void Save(FILE* fp);

        int getLength() { return len; }
        int getSize() { return sizeof(len) + getLength(); }
        void * getData() { return (void*)data; }
    private:
        Data() {}
        int len;
        char *  data;
    };
    std::list<Data*> mDataList;
};



