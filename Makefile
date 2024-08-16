CXX = g++
CXXFLAGS = -std=c++11 -Wall
SRCDIR = src
BINDIR = .

TARGET = csma
SOURCE = $(SRCDIR)/csma.cpp

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(BINDIR)/$(TARGET)