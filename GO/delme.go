package main

import "fmt"

func main() {
	// Declare an array
	arr := [5]int{10, 20, 30, 40, 50}

	// Make a slice from the array
	slice := arr[1:4] // Includes index 1, 2, 3 (not 4)

	fmt.Println("Array:", arr)
	fmt.Println("Slice:", slice)
}
