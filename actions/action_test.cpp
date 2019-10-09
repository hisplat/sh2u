

#include <map>
#include <string>
#include <stdlib.h>

#include "../request.h"
#include "../response.h"
#include "../tools/logging.h"

extern "C" void action_index(const simple_httpd::Request& request, simple_httpd::Response& response)
{
    const char * s = "Hello";
    response.content().append(s, strlen(s));
}

extern "C" void action_input_keyevent(const simple_httpd::Request& request, simple_httpd::Response& response)
{
    std::string key = request.getParameter("key");
    ILOG() << "input keyevent. key = " << key;
    if (key == "")
        return;

    key = std::string("input keyevent ") + key;
    response.content().append("OK", 2);
    system(key.c_str());
}



