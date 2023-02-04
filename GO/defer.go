package main

import "fmt"

var foo2 = func() func() {
    var state bool
    var count int
    return func() {
        count++
        fmt.Println("foo2() ", count)
        if !state {
            state = true
            fmt.Println("foo2() first")
        } else {
            fmt.Println("foo2() next")
        }
    }
}()

var state int
func foo() func() {
    state++
    fmt.Println("foo() state", state)
    if state == 1 {
    return func() {
         fmt.Println("bar() 1")
    }
    }
    if state == 2 {
    return func() {
         fmt.Println("bar() 2")
    }
    }
    if state == 3 {
    return func() {
         fmt.Println("bar() 3")
    }
    }
    if state == 4 {
    return func() {
         fmt.Println("bar() 4")
    }
    }
    if state == 5 {
    return func() {
         fmt.Println("bar() 5")
    }
    }
    return func() {
            mystate := state
            fmt.Println("bar() state", mystate)
    }
}

func main() {
   for i := 1; i < 5; i++ {
      bar := foo()
      defer bar()
      //defer foo2()
   }
   fmt.Println("Bye Bye")
}
