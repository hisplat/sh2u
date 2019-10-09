

CXX	:= g++
LD	:= g++
AR	:= ar
CXXFLAGS := -Wall -O2 -Werror -g -fPIC -Wno-unused-result
INCLUDES = -I../

TARGET	= libaction_test.so

LINKS	= -L. -shared -fPIC

SOURCES := action_test.cpp

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



