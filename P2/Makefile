CXXFLAGS = -g -Wall
#CXXFLAGS += -O2

CXXFLAGS += -I include/

SRC = $(wildcard src/*.cpp)
OBJ = $(addprefix obj/, $(notdir $(SRC:.cpp=.o)))

all: bridge.touch server client

bridge.touch:
	mkdir obj
	touch bridge.touch

server: $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

client: $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj/
	rm -f server
	rm -f client
	rm -f bridge.touch


.PHONY:
	all server client clean bridge.touch