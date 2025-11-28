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

## CLEANUP EVERYTHING FIRST
> Stop any running loader

If your loader is running:
Press Ctrl+C to stop it.

> Remove the old cgroup
**sudo rmdir /sys/fs/cgroup/mybpf**

> Kill old myprocess instances
 **sudo pkill myprocess**


# Explanation for Problem Statement 3:

## Explanation for the highlighted constructs
> cnp is the channel variable.

> chan func() is the buffered channel of length 10. It can carry functions with no parameters and no return values.

> make is used to initiate the channel.

> go func() { ... }() starts a new goroutine.

> for i := 0; i < 4; i++ { ... } runs 4 times and creates 4 worker goroutines.

> for f := range cnp { f() } receiving the channels using range.

> () at the end of func() { ... } calls the anonymous function immediately.

> func() { fmt.Println("HERE1") } is an anonymous function that will print "HERE1".

> cnp <- sends that function into the channel.

## Use Cases
> make(chan func(), 10) : Can be used to initialize a buffered channel.

> Workers (goroutines) :  is a thread which process and communicated using the channel and process the requests.

> for i := 0; i < 4; i++ { go func() { ... }() } : starting the 4 worker go routinues

> for f := range cnp { f() }: is to used retrive from channel

> cnp <- func() { fmt.Println("HERE1") } : Send the data to the channel

## What is the significance of FOR Loop with 4 iterations?
> The loop runs 4 times.

> In each iteration, a new goroutine (background worker) is created.

> These 4 workers all listen to the same channel cnp.

> When functions are sent into the channel, they can be shared among the 4 workers.

> This allows the work to be done in concurrently.

##  What is the significance of make(chan func(), 10)?
> It creates a channel named cnp that can hold functions.

> The number 10 is the buffer size, so it can store 10 functions without blocking.

> It provides a safe way for goroutines to send and receive functions during execution.

##  Why "HERE1" is not getting printed?
> The function which prints "HERE1" is sent into the channel.

> while the main thread started execution and printed "Hello" and then exits.

> The main thread doesnt wait for the goroutines, so gorountines doesnt complete the process so "HERE1" is not printed.

> We need to give some sleep like time.Sleep(10* time.Second) for "HERE1" to print.
