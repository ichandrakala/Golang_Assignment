# Steps to execute Problem statement1: Drop packets using eBPF
## Prerequisites

### 1. Install these packages:

> On Ubuntu/Debian

>> **sudo apt-get install clang llvm libbpf-dev linux-headers-$(uname -r) bpftool**

> Build the .o file (your existing command is correct)

>> **clang -O2 -g -target bpf -c xdp_drop_tcp.c -o xdp_drop_tcp.o**

### 2. Load the XDP Program Using ip(This will start dropping all TCP packets to port 4040)
Replace ens33 with your actual interface:

**sudo ip link set dev ens33 xdpgeneric obj xdp_drop_tcp.o sec xdp**
> Check:

**ip link show ens33**

> You should see:

xdpgeneric
prog/xdp id ...



# Steps to execute Problem statement2: Drop packets only for a given process
