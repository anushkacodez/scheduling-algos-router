# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++17

# Executable names (adding .exe for Windows)
TARGETS = islip.exe priority_queue_voq.exe rr_voq.exe wfq_voq.exe

# Compile all
all: $(TARGETS)

# Compile islip algorithm
islip.exe: islip.cpp
	$(CXX) $(CXXFLAGS) -o islip.exe islip.cpp

# Compile priority queue VOQ algorithm
priority_queue_voq.exe: priority_queue_voq.cpp
	$(CXX) $(CXXFLAGS) -o priority_queue_voq.exe priority_queue_voq.cpp

# Compile round-robin VOQ algorithm
rr_voq.exe: rr_voq.cpp
	$(CXX) $(CXXFLAGS) -o rr_voq.exe rr_voq.cpp

# Compile weighted fair queuing VOQ algorithm
wfq_voq.exe: wfq_voq.cpp
	$(CXX) $(CXXFLAGS) -o wfq_voq.exe wfq_voq.cpp

# Clean executables
clean:
	del /f /q islip.exe priority_queue_voq.exe rr_voq.exe wfq_voq.exe

