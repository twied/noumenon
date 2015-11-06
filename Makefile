CXX = g++

CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -g -O0

HFILES = $(wildcard src/*.h)
CFILES = $(wildcard src/*.cpp)
OFILES = $(patsubst %.cpp,%.o,$(CFILES))

all: noumenon

noumenon: $(OFILES)
	$(CXX) -o noumenon $^

$(OFILES): %.o : %.cpp $(HFILES)
	$(CXX) $(CXXFLAGS) -c -o $@ $(filter %.cpp,$<)

antlrworks-1.5.jar:
	wget http://www.antlr3.org/download/antlrworks-1.5.jar

clean:
	rm -f noumenon src/*.o

afl:
	make CXX=afl-g++ clean all
	echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
	-afl-fuzz -i examples -o findings ./noumenon @@
	echo ondemand | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

test:
	@for file in examples/*.nm; do \
		echo test $$file; ./noumenon "$$file" 2>&1 | diff -u tests/$$(basename "$$file" ".nm").expect - ; \
	done
	@echo done

.PHONY: clean afl test
