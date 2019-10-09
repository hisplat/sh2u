

CXX	:= g++
LD	:= g++
AR	:= ar
CXXFLAGS := -Wall -O2 -Werror -g
INCLUDES = -I../

TARGET	= a.out

LINKS	= -L.
LIBS	= -lpthread

SOURCES := test.cpp
SOURCES += reliable_udp_socket.cpp
SOURCES += socket.cpp
SOURCES += dump.cpp
SOURCES += timeout.cpp
SOURCES += lock.cpp
SOURCES += buffer.cpp
SOURCES += logging.cpp

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



