// xdp_drop_tcp.c
// Build: clang -O2 -g -target bpf -c xdp_drop_tcp.c -o xdp_drop_tcp.o

#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>          // for IPPROTO_TCP
#include <bpf/bpf_endian.h>    // for bpf_ntohs()


struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);       // single slot at key 0
    __type(key, __u32);
    __type(value, __u32);
} port_map SEC(".maps");

/* XDP program: drop TCP packets whose destination port == configured port */
SEC("xdp")
int xdp_drop_tcp(struct xdp_md *ctx)
{
    void *data = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;

    struct ethhdr *eth = data;
    if ((void*)(eth + 1) > data_end)
        return XDP_PASS;

    if (bpf_ntohs(eth->h_proto) != ETH_P_IP)
        return XDP_PASS;

    struct iphdr *ip = data + sizeof(*eth);
    if ((void*)(ip + 1) > data_end)
        return XDP_PASS;

    if (ip->protocol != IPPROTO_TCP)
        return XDP_PASS;

    __u32 ihl = ip->ihl * 4;
    struct tcphdr *tcp = (void*)ip + ihl;
    if ((void*)(tcp + 1) > data_end)
        return XDP_PASS;

    __u32 key = 0;
    __u32 *port_ptr = bpf_map_lookup_elem(&port_map, &key);
    if (!port_ptr)
        return XDP_PASS;

    __u16 dst_port = bpf_ntohs(tcp->dest);

    if (dst_port == ( __u16)(*port_ptr) ) {
        return XDP_DROP;
    }

    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
