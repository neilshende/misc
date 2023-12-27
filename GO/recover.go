package main

import (
  "fmt"
  "time"
)

func main() {
    defer func() {
        if r := recover(); r != nil {
            fmt.Println("Panic recovered:", r)
            // Handle the panic or log it
        }
    }()
  ch := make(chan string) // Channel for receiving messages

  // Launch a goroutine to send a message after 3 seconds
  go func() {
    defer func() {
        if r := recover(); r != nil {
            fmt.Println("Panic recovered:", r)
            // Handle the panic or log it
        }
    }()
    time.Sleep(3 * time.Second)
    ch <- "Hello!" //Panic. will be cought by recover of this thread.
  }()

  select {
  case message := <-ch: // Case for receiving from the channel
    fmt.Println("Received message:", message)
  case <-time.After(2 * time.Second): // Case for timeout after 2 seconds
    fmt.Println("Waiting for message timed out!")
    close(ch) // Close the channel to indicate we're done waiting
  }
  time.Sleep(5 * time.Second)
  ch <- "goodbye" //panic will be cought by recover of main thread.
  fmt.Println("exiting") // This statement will not be executed.
                  // We are toast, the recovery logic will not save us.
                  // other than exiting with 0 instead of bt and 1.
}
