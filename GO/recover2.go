package main

import (
  "fmt"
  "time"
)
func handlePanickingGoroutine(ch chan interface{}) {
    for {
        select {
        case p := <-ch:
            fmt.Println("Panic recovered from goroutine:", p)
            return
        case <-func() (CB chan bool) { // Enclose recover() in a function call
            CB = make(chan bool)
            if p := recover(); p != nil {
                fmt.Println("Panic recovered from goroutine:", p)
                CB<-true
                return
            }
            //CB<-false
            return
        }():
            // Nothing to do here, as panic handling is done within the function
        }
    }
// next for loop another implementation.
    for {
        select {
        case p := <-ch:
            if p != nil {
                fmt.Println("Panic recovered from goroutine:", p)
             }
        case <-time.After(time.Second):
           fmt.Println("Passed a second");

        }
    }
}

func main() {
    ch := make(chan interface{})
    ch2 := make(chan int)
    close(ch2)
    go func() {
        defer func() {
            if p := recover(); p != nil {
                fmt.Println("sending p to handlePanic:", p)
                ch <- p
            }
        }()

        time.Sleep(2*time.Second)
        // Code that might panic
        ch2 <- 911
    }()

    go handlePanickingGoroutine(ch)

    time.Sleep(5*time.Second)

}
