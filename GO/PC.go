package main

import (
	"fmt"
	"time"
)

func producer(ch chan int) {
	for i := 1; i <= 5; i++ {
		fmt.Printf("Sending %d\n", i)
		ch <- i
		time.Sleep(500 * time.Millisecond) // Simulate producing data
	}
	close(ch)
}

func consumer(ch chan int) {
	for val := range ch {
		fmt.Printf("Received %d\n", val)
		time.Sleep(1 * time.Second) // Simulate consuming data
	}
}

func main() {
	ch := make(chan int, 3) // Create a buffered channel with capacity 3

	go producer(ch)
	go consumer(ch)

	time.Sleep(6 * time.Second) // Wait for the program to finish
}
