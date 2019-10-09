

#define _CRT_NONSTDC_NO_DEPRECATE

#include "archive.h"
#include "serial.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

#include <assert.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace rapidxml;

namespace tools {

Archive::Archive()
{
    xml_node<> * node = doc.allocate_node(node_element, "Archive", "");
    doc.append_node(node);
}

Archive::~Archive()
{
    vector<const char *>::iterator  iter;
    for(iter = vecStr.begin(); iter != vecStr.end(); iter ++)
    {
        const char *p = *iter;
        if(p != NULL)
            free((void *)p);
    }
    doc.clear();
}

Archive::Archive(const Archive& o)
{
    Archive& s = (Archive&)o;
    fromStdString(s.toStdString());
}

Archive& Archive::operator=(const Archive& o)
{
    Archive& s = (Archive&)o;
    fromStdString(s.toStdString());
    return *this;
}

Archive& Archive::operator<<(int i)
{
    char    buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%d", i);
    return append(buffer, "int");
}

Archive& Archive::operator<<(bool b)
{
    char    buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    if(b)
        strcpy(buffer, "true");
    else
        strcpy(buffer, "false");
    return append(buffer, "bool");
}

Archive& Archive::operator>>(bool& b)
{
    string  s = first();
    if(strcmp(s.c_str(), "true") == 0)
    {
        b = true;
    } else {
        b = false;
    }
    return *this;
}

Archive& Archive::operator>>(int& i)
{
    i = atoi(first().c_str());
    return *this;
}

Archive& Archive::operator<<(float f)
{
    char    buffer[1024];
    sprintf(buffer, "%f", f);
    return append(buffer, "float");
}

Archive& Archive::operator<<(string str)
{
    return append(str.c_str(), "std::string");
}

Archive& Archive::operator<<(const char* str)
{
    return append(str, "cstring");
}

Archive& Archive::operator<<(long l)
{
    char    buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%ld", l);
    return append(buffer, "long");
}

Archive& Archive::operator<<(unsigned int i)
{
    char    buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%u", i);
    return append(buffer, "unsigned int");
}

Archive& Archive::operator<<(unsigned long l)
{
    char    buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%lu", l);
    return append(buffer, "unsigned long");
}

Archive& Archive::operator>>(float& f)
{
    f = (float)atof(first().c_str());
    return *this;
}

Archive& Archive::operator>>(string& str)
{
    str = first();
    return *this;
}

Archive& Archive::operator>>(long& l)
{
    l = atol(first().c_str());
    return *this;
}

Archive& Archive::operator>>(unsigned int& i)
{
    int     t = atoi(first().c_str());
    i = (unsigned int)t;
    return *this;
}

Archive& Archive::operator>>(unsigned long& l)
{
    long    t = atol(first().c_str());
    l = (unsigned long)t;
    return *this;
}

// �ݹ飡
void Archive::CopyXml(xml_node<> * dst, xml_node<> * src)
{
    xml_node<> *            noded;
    xml_node<> *            nodes;
    xml_attribute<> *       attrd;
    xml_attribute<> *       attrs;
    const char *            pn;
    const char *            pv;

    pn = strdup(src->name());
    pv = strdup(src->value());

    noded = dst->document()->allocate_node(node_element, pn, pv);
    for(attrs = src->first_attribute(); attrs != NULL; attrs = attrs->next_attribute())
    {
        pn = strdup(attrs->name());
        pv = strdup(attrs->value());
        vecStr.push_back(pn);
        vecStr.push_back(pv);
        attrd = dst->document()->allocate_attribute(pn, pv);
        noded->append_attribute(attrd);
    }
    dst->append_node(noded);

    if(strcmp(src->name(), "Archive") == 0)
    {
        for(nodes=src->first_node(); nodes != NULL; nodes = nodes->next_sibling())
        {
            CopyXml(noded, nodes);
        }
    }
}

Archive& Archive::append(const char * value, const char * type)
{
    xml_node<> * head = doc.first_node("Archive");
    if(head == NULL)
        return *this;
    const char * p1, *p2;
    p1 = strdup(value);
    p2 = strdup(type);
    vecStr.push_back(p1);
    vecStr.push_back(p2);

    if(strcmp(p2, "Archive") == 0)
    {
        xml_document<>       dc;
        dc.parse<0>((char *)p1);
        xml_node<> * hd;
        hd = dc.first_node("Archive");
        CopyXml(head, hd);
    } else {
        xml_node<> * node = doc.allocate_node(node_element, "item", p1);
        head->append_node(node);
        xml_attribute<> * attr = doc.allocate_attribute("type", p2);
        node->append_attribute(attr);
    }
    return *this;
}

xml_node<> * Archive::GetHead()
{
    xml_node<> *    head = doc.first_node();
    assert(head != NULL);
    return head;
}

string Archive::first(void)
{
    string          str;
    Archive        ar;
    xml_node<> * head = doc.first_node("Archive");

    if(head == NULL)
        return string("");
    xml_node<> * node = head->first_node();
    if(node != NULL)
    {
        str = node->value();
        head->remove_node(node);
    }
    return str;
}

string Archive::toStdString(void)
{
    string  s;
    print(std::back_inserter(s), doc, 0);
    return s;
}

Archive& Archive::fromStdString(string xml)
{
    string          tmp(xml.c_str());
    xml_document<>  dc;
    dc.parse<0>((char *)tmp.c_str());
    xml_node<> * hd = dc.first_node("Archive");
    if(hd == NULL) {
        throw("not Archive!");
        return *this;
    }
    xml_node<> * head = doc.first_node("Archive");
    if(head == NULL) {
        throw("not correct!");
        return *this;
    }

    head->remove_all_nodes();

    xml_node<> * nd;
    for(nd = hd->first_node(); nd != NULL; nd = nd->next_sibling())
    {
        CopyXml(head, nd);
    }
    return *this;
}

Archive& Archive::operator<<(Archive& o)
{
    append(o.toStdString().c_str(), "Archive");
    return *this;
}

Archive& Archive::operator>>(Archive& o)
{
    xml_node<> * head = doc.first_node("Archive");

    if(head == NULL)
        return *this;

    xml_node<> * node = head->first_node();
    xml_node<> * nd;
    if(node != NULL)
    {
        if(strcmp(node->name(), "Archive") == 0)
        {
            xml_node<> *    hd = o.GetHead();
            if(hd == NULL)
            {
                head->remove_node(node);
                return *this;
            }
            for(nd = node->first_node(); nd != NULL; nd= nd->next_sibling())
            {
                o.CopyXml(hd, nd);
            }
            head->remove_node(node);
        } else {
            head->remove_node(node);
        }
    }
    return *this;
}

Archive& Archive::operator<<(Serial& o)
{
    o.Serialize(*this);
    return *this;
}

Archive& Archive::operator>>(Serial& o)
{
    o.Deserialize(*this);
    return *this;
}

Archive& Archive::fromXmlNode(xml_node<> * node)
{
    xml_node<> * head       = GetHead();
    head->remove_all_nodes();

    if(strcmp(node->name(), "Archive") == 0)
    {
        xml_node<> *    nd;
        for(nd=node->first_node(); nd != NULL; nd=nd->next_sibling())
        {
            CopyXml(head, nd);
        }
    } else {
        CopyXml(head, node);
    }
    return *this;
}

void Archive::clear(void)
{
    xml_node<> * head       = GetHead();
    if(head == NULL)
        return;
    head->remove_all_nodes();
}

bool Archive::empty(void)
{
    xml_node<> * head       = GetHead();
    if(head == NULL)
        return true;
    xml_node<> * node = head->first_node();
    if(node == NULL)
        return true;
    return false;
}

} // namespace tools

std::basic_ostream<char>& operator <<(std::basic_ostream<char>& out, tools::Archive& ar)
{
    out << ar.toStdString();
    return out;
}



