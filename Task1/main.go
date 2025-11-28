package main

import (
	"fmt"
	"net/http"
)

func main() {
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintln(w, "Hello from port 4040!")
	})

	fmt.Println("Server running on http://localhost:4040")
	if err := http.ListenAndServe(":4040", nil); err != nil {
		panic(err)
	}
}
