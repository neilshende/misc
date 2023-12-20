package main

import (
    "fmt"
    "time"
)

var (

    ccc chan bool
    ddd chan bool
)

func main() {
    ccc = make(chan bool)
    ddd = make(chan bool)
    go func() {
       fmt.Printf("server waiting for client to be ready\n")
       <-ccc
       fmt.Printf("server received client ready \n")
       ddd <- true
    } ()
    go func() {
       fmt.Printf("client not yet ready\n")
       time.Sleep(8 * time.Second)
       fmt.Printf("client ready\n")
       ccc <- true
    } ()
    fmt.Printf("main waiting\n")
    <-ddd
    fmt.Printf("main done\n")
}

