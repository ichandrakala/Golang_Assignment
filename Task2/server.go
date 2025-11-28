package main

import (
    "fmt"
    "net"
)

func main() {
    port := "4040"
    addr := ":" + port

    listener, err := net.Listen("tcp", addr)
    if err != nil {
        fmt.Println("Failed to start server:", err)
        return
    }
    fmt.Println("Go TCP Server listening on port", port)

    for {
        conn, err := listener.Accept()
        if err != nil {
            fmt.Println("Accept error:", err)
            continue
        }

        fmt.Println("Accepted a connection from:", conn.RemoteAddr())
        conn.Close()
    }
}
