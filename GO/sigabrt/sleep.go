package main
// #cgo CFLAGS: -g -Wall
// #include <stdlib.h>
// #include "greeter.h"
import "C"
import (
    "fmt"
    "unsafe"
    "os"
    "os/signal"
    "strings"
    "syscall"
    "time"
//    "golang.org/x/sys/unix"
)
 
func main() {
    // Print the process ID to make it easier to send this program a signal.
    fmt.Printf("pid: %d\n", os.Getpid())
 
    // If this program's first argument is "trap" then trap SIGABRT.
    if len(os.Args) > 1 && strings.EqualFold(os.Args[1], "trap") {
        n := make(chan os.Signal, 1)
        signal.Notify(n, syscall.SIGABRT)
        go func() {
            for sig := range n {
                fmt.Println(sig)
                os.Exit(1)
            }
        }()
    }
 
    // If this program's first argument is "trap" then trap SIGABRT.
    if len(os.Args) > 1 && strings.EqualFold(os.Args[1], "dump") {
        signal.Reset(syscall.SIGABRT)
    }

    f, err := os.OpenFile("./error.log", os.O_WRONLY|os.O_CREATE|os.O_APPEND, 0666)
    defer f.close()

  //  stderrFd := int(os.Stderr.Fd())
  //  oldfd, err := unix.Dup(stderrFd)
  //  err = unix.Dup2(int(f.Fd()), stderrFd)

   C.greet()
 
    time.Sleep(1000 * time.Second)
}
