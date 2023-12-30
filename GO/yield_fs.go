package main

import (
    "fmt"
)

func fibonacci(c chan int, quit chan bool) {
    a, b := 1, 1
    for {
        select {
        case c <- a:
            a, b = b, a+b
        case <-quit:
            return
        }
    }
}

func main() {
    c := make(chan int)
    quit := make(chan bool)
    go fibonacci(c, quit)

    for f := <-c; f>0; f = <-c {
        fmt.Println(f)
    }

    quit <- true // Signal the goroutine to stop
}
