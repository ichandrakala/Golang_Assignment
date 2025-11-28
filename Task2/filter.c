// SPDX-License-Identifier: GPL-2.0
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

#define ALLOWED_PORT 4040
// process name to match (short; 'comm' is 16 bytes including NUL)
const volatile char target_comm[16] = "myprocess";

SEC("cgroup/connect4")
int handle_connect4(struct bpf_sock_addr *ctx)
{
    // Only consider TCP stream sockets
    if (ctx->type != SOCK_STREAM)
        return 1; // allow other socket types

    // get current process name
    char comm[16];
    if (bpf_get_current_comm(&comm, sizeof(comm)) != 0)
        return 1; // if helper fails, be permissive

    // Compare comm with target_comm (safe memcmp-like loop)
    int match = 1;
    #pragma unroll
    for (int i = 0; i < 16; i++) {
        if (comm[i] != target_comm[i]) {
            match = 0;
            break;
        }
        // if we've hit the terminating NUL in both, still match
        if (comm[i] == 0)
            break;
    }

    if (!match)
        return 1; // other processes: allow

    // This is the target process; allow only destination port ALLOWED_PORT
    // ctx->user_port is in network byte order
    __u16 dst = ctx->user_port;
    if (dst == bpf_htons(ALLOWED_PORT))
        return 1; // allow

    // otherwise deny the connect
    return 0;
}

char LICENSE[] SEC("license") = "GPL";
