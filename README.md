# Steps to execute Problem statement1: Drop packets using eBPF
## Prerequisites

### Install these packages:

> On Ubuntu/Debian
**sudo apt-get install clang llvm libbpf-dev linux-headers-$(uname -r) bpftool**

> Build the .o file (your existing command is correct)
**clang -O2 -g -target bpf -c xdp_drop_tcp.c -o xdp_drop_tcp.o**

# Steps to execute Problem statement2: Drop packets only for a given process
