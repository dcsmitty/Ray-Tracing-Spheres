CXX       = g++
CXXFLAGS  = -g -O2 -c
LDFLAGS   = -g -O2 -lGL -lglut
RM        = rm -f
EXEC      = ray_tracer

all: $(EXEC)

$(EXEC): ray_tracer.o main.o
	$(CXX) $(LDFLAGS) $^ -o $@

ray_tracer.o: ray_tracer.cpp ray_tracer.h
	$(CXX) $(CXXFLAGS) $< -o $@

main.o: main.cpp ray_tracer.h
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	$(RM) *~ *.bak *.o $(EXEC)
