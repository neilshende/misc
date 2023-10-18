package main

import (
	"fmt"
	"os"
	"os/signal"
	"syscall"
)

func multiSignalHandler(signal os.Signal) {

	switch signal {
	case syscall.SIGHUP:
		fmt.Println("Signal:", signal.String())
		os.Exit(0)
	case syscall.SIGINT:
		fmt.Println("Signal INT VIVEK:", signal.String())
		os.Exit(0)
	case syscall.SIGTERM:
		fmt.Println("Signal TERM VIVEK:", signal.String())
		os.Exit(0)
	case syscall.SIGQUIT:
		fmt.Println("Signal:", signal.String())
		os.Exit(0)
	default:
		fmt.Println("Unhandled/unknown signal")
	}
}

func main() {
	sigchnl := make(chan os.Signal, 1)
	signal.Notify(sigchnl, os.Interrupt, syscall.SIGHUP, syscall.SIGINT, syscall.SIGTERM) //we can add more sycalls.SIGQUIT etc.
	exitchnl := make(chan int)

	go func() {
		for {
			s := <-sigchnl
			fmt.Println("CALLING HND for:", s.String())
			multiSignalHandler(s)
		}
	}()

	exitcode := <-exitchnl
	fmt.Println("Do we ever reach here?, and how?")
	os.Exit(exitcode)
}
