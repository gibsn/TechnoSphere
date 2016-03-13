CXXFLAGS = -g -Wall
#CXXFLAGS += -O2

CXXFLAGS += -I include/

#SRC = $(wildcard src/*.cpp)
SRC_S = clients.cpp server.cpp
SRC_C = client.cpp
OBJ_S = $(addprefix obj/, $(notdir $(SRC_S:.cpp=.o)))
OBJ_C = $(addprefix obj/, $(notdir $(SRC_C:.cpp=.o)))
DEPS = $(addprefix deps/, $(notdir $(SRC:.cpp=.d)))

src_to_obj = $(addprefix obj/, $(notdir $(1:.cpp=.o)))

all: chatsrv client

ifneq ($(MAKECMDGOALS), clean)
-include bridge.touch
endif

chatsrv: $(OBJ_S)
	$(CXX) $(CXXFLAGS) $^ -o $@

client: $(OBJ_C)
	$(CXX) $(CXXFLAGS) $^ -o $@

deps/%.d: src/%.cpp
	$(CXX) $(CXXFLAGS) -E -MM -MT $(call src_to_obj, $<) -MT $@ -MF $@ $<

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

bridge.touch:
	mkdir obj
	mkdir deps
	echo "-include $(DEPS)" > $@

clean:
	rm -rf obj/
	rm -rf deps/
	rm -f chatsrv
	rm -rf chatsrv.dSYM
	rm -f client
	rm -rf client.dSYM
	rm -f bridge.touch

.PHONY:
	all clean