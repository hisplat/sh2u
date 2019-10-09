

CXX	:= g++
LD	:= g++
AR	:= ar
CXXFLAGS := -Wall -O2 -Werror -g -fPIC -rdynamic
INCLUDES = -I../ -fPIC

TARGET	= a.out

LINKS	= -L. -ldl -rdynamic
LIBS	= -lpthread

SOURCES := main.cpp
SOURCES += client.cpp
SOURCES += http_header.cpp
SOURCES += httpd_config.cpp
SOURCES += request.cpp
SOURCES += response.cpp
SOURCES += tools/socket.cpp
SOURCES += tools/select.cpp
SOURCES += tools/string_util.cpp
SOURCES += tools/logging.cpp
SOURCES += tools/dump.cpp
SOURCES += tools/buffer.cpp
SOURCES += tools/serialize/archive.cpp

OBJS := $(SOURCES:.cpp=.o)
DEPS := $(SOURCES:.cpp=.d)

all: prebuild $(TARGET)

$(TARGET): $(OBJS) $(DEPS)
	@echo Linking $@ ...
	$(LD) $(OBJS) $(LINKS) $(LIBS) -o$@
	@echo -------------------------------------------
	@echo done.

.cpp.o:
	@echo Compling $@ ...
	$(CXX) -c $< $(INCLUDES) $(CXXFLAGS)  -o $@
	@echo -------------------------------------------

%.d:%.cpp
	$(CXX) -MM $< $(INCLUDES) $(CXXFLAGS) -o $@

prebuild:
	#echo -------

include $(DEPS)

clean:
	rm -fr $(OBJS) $(DEPS) $(TARGET) 



