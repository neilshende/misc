package main

import (
	"fmt"
	"time"
)

func modifyArray(arr [3]int) {
	arr[0] = 100
	fmt.Println("Inside goroutine (array):", arr)
}

func modifySlice(slc []int) {
	slc[0] = 200
	fmt.Println("Inside goroutine (slice):", slc)
}

func main() {
	arr := [3]int{1, 2, 3}
	slc := []int{1, 2, 3}

	go modifyArray(arr)
	go modifySlice(slc)

	// Give goroutines time to complete
	time.Sleep(100 * time.Millisecond)

	fmt.Println("After goroutines:")
	fmt.Println("Main array:", arr)   // Not modified
	fmt.Println("Main slice:", slc)   // Modified

	modifySlice(arr[0:3])
	fmt.Println("Main array:", arr)   // Not modified
}
