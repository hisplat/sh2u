// CConfig ��
//
// ���ڴ�������M$��ini��ʽ���ļ���
// ���������˿ɴ�����[section]�������ļ���
//

#ifndef __CCONFIG_H__
#define __CCONFIG_H__
#pragma once

#include <string>
#include <map>
using namespace std;

// ö�������ֶεĻص�������
// ���� false ����ֹö�١�
typedef bool (* lpfnConfigEnum)(const char * section, const char * name, const char * value, void * param);

namespace tools {
class CConfig {
public:
        CConfig();
        ~CConfig();

        // ���������ļ���
        bool    Load(const char * filename);
        bool    LoadBuffer(const char * buffer);

        // ���������ļ���
        void    Save(const char * filename);

        // ���ֶΡ�[section] -> name
        string  Read(string section, string name);

        // д�ֶ�
        void    Write(string section, string name, string value);

        // ɾ���ֶ�
        void    Remove(string section, string name);

        // ����[section]���ֶΡ�
        string  Read(string name);

        // д��[section]���ֶΡ�
        void    Write(string name, string value);

        // ɾ����[section]���ֶΡ�
        void    Remove(string name);

        // ö�������ļ������е��ֶΡ�
        void    Enum(lpfnConfigEnum proc, void * param);

private:
        map<string, map<string, string> >       m_map;
};

}
#endif

