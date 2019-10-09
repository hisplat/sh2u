// CConfig 类
//
// 用于处理类似M$的ini格式的文件。
// 额外增加了可处理无[section]的配置文件。
//

#ifndef __CCONFIG_H__
#define __CCONFIG_H__
#pragma once

#include <string>
#include <map>
using namespace std;

// 枚举所有字段的回调函数。
// 返回 false 则中止枚举。
typedef bool (* lpfnConfigEnum)(const char * section, const char * name, const char * value, void * param);

namespace tools {
class CConfig {
public:
        CConfig();
        ~CConfig();

        // 加载配置文件。
        bool    Load(const char * filename);
        bool    LoadBuffer(const char * buffer);

        // 存入配置文件。
        void    Save(const char * filename);

        // 读字段。[section] -> name
        string  Read(string section, string name);

        // 写字段
        void    Write(string section, string name, string value);

        // 删除字段
        void    Remove(string section, string name);

        // 读无[section]的字段。
        string  Read(string name);

        // 写无[section]的字段。
        void    Write(string name, string value);

        // 删除无[section]的字段。
        void    Remove(string name);

        // 枚举配置文件中所有的字段。
        void    Enum(lpfnConfigEnum proc, void * param);

private:
        map<string, map<string, string> >       m_map;
};

}
#endif

