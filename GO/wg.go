package main

import (
    "fmt"
    "sync"
    "time"
    "math"
    "strconv"
)

func worker(id int) {
    fmt.Printf("Worker %d starting\n", id)

    time.Sleep(time.Second)
    fmt.Printf("Worker %d done\n", id)
}

func main() {

   n := make(map[int64]int64)
   n[1]=2
   n[2]=4
   for a, b := range n {
     fmt.Printf("A, B is %d, %d \n", a, b)
   }
   v, ok := n[3]
   if !ok {
      fmt.Printf("Not OK %s\n", strconv.Itoa(math.MaxInt64))
   } else {
      fmt.Printf("V is \n", v)
   }

    var wg sync.WaitGroup

    for i := 1; i <= 5; i++ {
        wg.Add(1)

        //i := i

        go func(x int) {
            defer wg.Done()
            worker(x)
        }(i)
    }

    wg.Wait()

}
