
# ram_speed
by rigaya

This is an attempt to measure RAM and cache performace.

It will run performace tests below.
- intercore latency tests
- RAM/Cache latency tests
- RAM/Cache bandwidth tests 

Please note that this app is sensitive to background tasks, and might not get expected results.

## System Requirements
Windows 8.1/10 (x64)  
Linux (x64)  

## Results
### RAM/Cache bandwidth MT
Result of the bandwidth test, using all physical cores on the CPU.
![RAM/Cache bandwidth MT](results/ram_cache_bandwidth_MT.png) 

### RAM/Cache bandwidth ST
Result of the bandwidth test, using a single cores on the CPU.
![RAM/Cache bandwidth ST](results/ram_cache_bandwidth_ST.png) 

### RAM bandwidth
Result of the bandwidth test, checking the bandwidth of the largest size tested.
![RAM bandwidth](results/ram_bandwidth.png) 

### RAM/Cache latency cachline random
![RAM/Cache bandwidth](results/ram_cache_latency_cacheline_random.png) 

### RAM/Cache latency full random
![RAM/Cache bandwidth](results/ram_cache_latency_full_random.png)

## To Build
### Windows
VC++2019 & nasm is required.

### Linux
C++14 compiler is required, and also nasm is recommended.  

However, it is possible to build without nasm, by changing ENABLE_ASM to 0 in makefile. In that case, using clang is recommended. 

## Precautions for using ram_speed
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.  

## License
The MIT License will be applied. 