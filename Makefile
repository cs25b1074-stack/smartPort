CXX      = c++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic

SRCS = main.cpp Container.cpp Manifest.cpp Ship.cpp Berth.cpp Crane.cpp \
       YardSlot.cpp ContainerYard.cpp HandlingJob.cpp ContainerRegistry.cpp \
       PortScheduler.cpp PortTime.cpp PortStatistics.cpp PortController.cpp
OBJS = $(SRCS:.cpp=.o)

smartport: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp *.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) smartport

.PHONY: clean
