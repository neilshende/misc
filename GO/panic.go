package main

import "fmt"

func main() {
    i := f()
    fmt.Println("Returned normally from f. ", i)
}

func f() int {
    defer func() {
        if r := recover(); r != nil {
            fmt.Println("Recovered in f", r)
            return
        }
    }()
    //fmt.Println("Calling g.")
    //g(0)
    panic(fmt.Sprintf("%v", 911))
    fmt.Println("Returned normally from g.")
    return 666
}

func g(i int) {
    if i > 3 {
        fmt.Println("Panicking!")
        panic(fmt.Sprintf("%v", i))
    }
    defer fmt.Println("Defer in g", i)
    fmt.Println("Printing in g", i)
    g(i + 1)
}
