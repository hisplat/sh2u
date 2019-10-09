

#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <list>
#include "rapidxml/rapidxml.hpp"

namespace tools {

class Serial;
class Archive
{
public:
        Archive();
        ~Archive();

        Archive(const Archive& o);
        Archive& operator=(const Archive& o);

        Archive& operator<<(int i);
        Archive& operator<<(float f);
        Archive& operator<<(std::string str);
        Archive& operator<<(const char* str);
        Archive& operator<<(long l);
        Archive& operator<<(unsigned int i);
        Archive& operator<<(unsigned long l);
        Archive& operator<<(bool b);

        Archive& operator>>(int& i);
        Archive& operator>>(float& f);
        Archive& operator>>(std::string& str);
        Archive& operator>>(long& l);
        Archive& operator>>(unsigned int& i);
        Archive& operator>>(unsigned long& l);
        Archive& operator>>(bool& b);

        Archive& operator<<(Archive& o);
        Archive& operator>>(Archive& o);

        Archive& operator<<(Serial& o);
        Archive& operator>>(Serial& o);

        template <typename T>
        Archive& operator<<(std::list<T>& o) {
            operator<<(o.size());
            typename std::list<T>::iterator it;
            for (it = o.begin(); it != o.end(); it++) {
                operator<<(*it);
            }
            return *this;
        }

        template <typename T>
        Archive& operator>>(std::list<T>& o) {
            int i, size;
            T   t;
            operator>>(size);
            o.clear();
            for (i = 0; i < size; i++) {
                operator>>(t);
                o.push_back(t);
            }
            return *this;
        }

        std::string    toStdString(void);
        Archive& fromStdString(std::string xml);
        Archive& fromXmlNode(rapidxml::xml_node<> * node);

        void      clear(void);
        bool      empty(void);

private:
        Archive&       append(const char * value, const char * type);
        std::string     first(void);
        rapidxml::xml_node<> *    GetHead();

        rapidxml::xml_document<>          doc;
        std::vector<const char *>    vecStr;

        void    CopyXml(rapidxml::xml_node<> * dst, rapidxml::xml_node<> * src); 
};

} // namespace tools

std::basic_ostream<char>& operator <<(std::basic_ostream<char>& out, tools::Archive& ar);






