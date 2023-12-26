package main

import (
    "fmt"
    "time"
)

func main() {
    c := make(chan int)
    go func () {
    for {  // send random sequence of bits to c
	select {
	case c <- 0:  // note: no statement, no fallthrough, no folding of cases
	case c <- 1:
	}
    }
    } ()

    for {
        i := <-c
        fmt.Printf("%d ", i);
        //time.Sleep(1 * time.Second)
    }

        time.Sleep(1 * time.Second)
}

