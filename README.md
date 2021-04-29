# Operating Systems:
- macOS Mojave v 10.14.6 (local computer)
- Ubuntu 20.04 (LTS) x64 (remote server)

# Compilation
- g++ main.cpp -o main -std=c++11 -lpthread
- g++ main.cpp -o main -std=c++11 -lpthread -g (to enable debugging)

# Running
1. Compile main.cpp
2. In the same directory, run ./main \<resources> \<threads>

# Testing
1. Largest thread/resource ==> 50/50 (I didn't test any higher than this, but maybe it'll do more?)
2. Edge Cases

    a. (1 resources, 20 thread) ==> succeeds
    
    b. (20 resources, 1 thread) ==> succeeds