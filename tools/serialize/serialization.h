
#pragma once

#include "archive.h"

class Serialization
{
public:
    Serialization() {}
    virtual ~Serialization() {}

    virtual void Serialize(Archive& ar) = 0;
    virtual void Deserialize(Archive& ar) = 0;

    int toBuffer(char * buffer, int len) {
        Archive ar;
        Serialize(ar);
        return ar.toBuffer(buffer, len);
    }

    void fromBuffer(const char* buffer, int len) {
        Archive ar;
        if (ar.fromBuffer(buffer, len))
            Deserialize(ar);
    }

    void SaveToFile(const char * file) {
        Archive ar;
        Serialize(ar);
        ar.Save(file);
    }

    void LoadFromFile(const char * file) {
        Archive ar;
        if (ar.Load(file))
            Deserialize(ar);
    }
};

class ArchiveSerializationFunctor {
public:
    ArchiveSerializationFunctor(Archive& a) : ar(a) {}
    void operator()(Serialization* b) {
        ar << *b;
    }
    void operator()(Serialization& b) {
        ar << b;
    }
private:
    Archive& ar;
};

class ArchiveDeserializationFunctor {
public:
    ArchiveDeserializationFunctor(Archive& a) : ar(a) {}
    void operator()(Serialization* b) {
        ar >> *b;
    }
    void operator()(Serialization& b) {
        ar >> b;
    }
private:
    Archive& ar;
};


