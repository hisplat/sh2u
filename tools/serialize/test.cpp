
#include "serialization.h"
#include <list>
#include <algorithm>
#include "../dump.h"

class B : public Serialization {
public:
    B() : i(0) {}
    virtual void Serialize(Archive& ar) {
        ar << i;
    }
    virtual void Deserialize(Archive& ar) {
        ar >> i;
    }
    void dump() {
        printf("B:i  = %d\n", i);
    }
    int i;
};


class A : public Serialization {
public:
    A () {
        B* b = new B();
        mB.push_back(b);
    }

    virtual void Serialize(Archive& ar) {
        ar << str;

        ArchiveSerializationFunctor asf(ar);
        for_each(mB.begin(), mB.end(), asf);

        ar << f << d;
    }

    virtual void Deserialize(Archive& ar) {
        ar >> str;

        ArchiveDeserializationFunctor adf(ar);

        std::list<B*>::iterator it;
        it = mB.begin();
        adf(*it);

        ar >> f >> d;
    }

    void dump() {
        printf("%s, %f, %f\n", str.c_str(), f, d);
        std::list<B*>::iterator it;
        it = mB.begin();
        (*it)->dump();
    }

    void change() {
        str += "changed.";
        f++;
        d++;
        std::list<B*>::iterator it;
        it = mB.begin();
        (*it)->i++;
    }
private:
    std::string str;
    std::list<B*>   mB;
    float f;
    double d;

};


class C : public Serialization {
public:
    int a;
    int b;
    int c;
    char    d[20];

    C() : a(2), b(3), c(4) {
        strcpy(d, "hello");
    }

    void reset() {
        a = 0;
        b = 0;
        c = 0;
        memset(d, 0, sizeof(d));
    }

    virtual void Serialize(Archive& ar) {
        ar << a << b << c << d;
    }

    virtual void Deserialize(Archive& ar) {
        ar >> a >> b  >> c >> d;
    }
};

int main()
{
    C   a;
    tools::dump(&a, sizeof(a));
    char    buffer[1024];
    int ret = a.toBuffer(buffer, sizeof(buffer));

    tools::dump(buffer, ret);

    C b;
    b.reset();

    b.fromBuffer(buffer, ret);
    tools::dump(&b, sizeof(b));
    printf("%d, %d, %d\n", b.a, b.b, b.c);
    printf("%s\n", b.d);

    return 0;
}










