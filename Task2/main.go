package main

import (
	"flag"
	"fmt"
	"log"
	"os"

	"github.com/cilium/ebpf"
	"github.com/cilium/ebpf/link"
	"os/signal"
	"syscall"
)

func main() {
	var (
		objPath   = flag.String("obj", "filter.o", "path to compiled BPF object")
		cgroupPath = flag.String("cgroup", "/sys/fs/cgroup/mybpf", "cgroup v2 path to attach the program")
	)
	flag.Parse()

	if _, err := os.Stat(*objPath); os.IsNotExist(err) {
		log.Fatalf("eBPF object not found: %s", *objPath)
	}

	// load spec
	spec, err := ebpf.LoadCollectionSpec(*objPath)
	if err != nil {
		log.Fatalf("failed to load eBPF spec: %v", err)
	}

	coll, err := ebpf.NewCollection(spec)
	if err != nil {
		log.Fatalf("failed to create eBPF collection: %v", err)
	}
	defer coll.Close()

	// program symbol name — adjust if your compiled object uses a different name
	prog := coll.Programs["handle_connect4"]
	if prog == nil {
		log.Fatalf("program handle_connect4 not found in %s; available programs: %v", *objPath, coll.Programs)
	}

	// prepare cgroup attach options
	// Attach type for connect4 is ebpf.AttachCGroupInet4Connect
	// (older/newer versions of the library might require AttachTypeForPlatform)
	opts := link.CgroupOptions{
		Path:    *cgroupPath,
		Attach:  ebpf.AttachCGroupInet4Connect,
		Program: prog,
	}

	l, err := link.AttachCgroup(opts)
	if err != nil {
		log.Fatalf("failed to attach program to cgroup %s: %v", *cgroupPath, err)
	}
	defer l.Close()

	fmt.Printf("attached program to cgroup %s\n", *cgroupPath)
	fmt.Println("Press Ctrl+C to exit...")
	sig := make(chan os.Signal, 1)
        signal.Notify(sig, syscall.SIGINT, syscall.SIGTERM)
        <-sig

        fmt.Println("Exiting...")
}
