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

### 3. Configure the Port in the eBPF Map

**sudo bpftool map show**
> Look for the map named port_map and note its ID.
> Update with that ID as shown below.

**sudo bpftool map update id 67 key 0 0 0 0 value 0xc8 0x0f 0x00 0x00**
> Check where map dumped with port or not
**sudo bpftool map dump id 67**
> you should see as shown below

[{
        "key": 0,
        "value": 4040
    }
]

### 4. Run the specific process (myprocess)

> Test allowed port passes, other ports drop

**go build -o myprocess myserver.go
./myprocess**

### 5. Detach XDP Program (This means → packet dropping stopped)

> If you want to remove:

**sudo ip link set dev ens33 xdp off**

> Check:

**ip link show ens33**

> You should now see:

(no xdp)


# Steps to execute Problem statement2: Drop packets only for a given process
## 1️ CREATE a fresh cgroup

**sudo mkdir -p /sys/fs/cgroup/mybpf**

> Verify:
**ls /sys/fs/cgroup/mybpf**

## 2️ Run the eBPF LOADER (this MUST stay running)

> Go to your project folder:
**cd ~/ebpf-drop/Task2**

> Run:
**sudo /usr/local/go/bin/go run main.go\
  -obj filter.o \
  -cgroup /sys/fs/cgroup/mybpf**

> Expected output:
attached program to cgroup /sys/fs/cgroup/mybpf
Press Ctrl+C to exit...

DO NOT PRESS CTRL+C
Leave it running!

## 3️ Open a NEW terminal window
> Put this NEW terminal’s shell into the cgroup
**echo $$ | sudo tee /sys/fs/cgroup/mybpf/cgroup.procs**

> Verify:
**cat /sys/fs/cgroup/mybpf/cgroup.procs**

You should see SHELL'S PID.

## 4 Run myprocess

**gcc myprocess.c -o myprocess**

> In this same terminal:

**./myprocess 8080**

EXPECTED (BLOCKED by eBPF):
connect: Permission denied

Then test allowed port:
**./myprocess 4040**

EXPECTED:
connect: Connection refused 
(Allowed by eBPF but no server listening)
connect: Success 
(Allowed by eBPF when server is listening)
