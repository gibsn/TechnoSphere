TEST_FILES = ../thirdparty/gtest/gtest-all.cc ../thirdparty/gtest/gtest_main.cc
SRC = allocator.cpp allocator_test.cpp
HDR = allocator.h
CXXFLAGS = -g -std=c++11
CXXFLAGS += -lpthread
CXXFLAGS += -I../thirdparty $(TEST_FILES)


all: tests.done

allocator_test: $(SRC) $(HDR)
	g++ $(CXXFLAGS) $(SRC) -o $@

tests.done: allocator_test
	./allocator_test
	touch tests.done

clean:
	rm -f tests.done
	rm -rf allocator_test.dSYM
	rm -f allocator_test

PHONY:
	all clean