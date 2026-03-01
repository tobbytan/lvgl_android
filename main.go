package main

import (
	"dotos/ui"
	"fmt"
	"log"
	"os"
	"os/exec"
	"os/signal"
	"syscall"
)

func runCmd(cmdline string) {
	log.Printf("run cmd: %s", cmdline)
	cmd := exec.Command("sh", "-c", cmdline)
	err := cmd.Run()
	if err != nil {
		log.Printf("run cmd error: %s", err)
	}
	log.Printf("run cmd done")

}
func main() {

	// runCmd("setprop ctl.stop zygote")
	// time.Sleep(time.Second * 1)
	// runCmd("setprop ctl.stop bootanim")

	ui.InitUI()

	ch := make(chan os.Signal, 1)
	signal.Notify(ch, os.Interrupt, syscall.SIGTERM)

	sig := <-ch
	fmt.Println("Got signal:", sig)

	// runCmd("setprop ctl.start bootanim")
	// runCmd("setprop ctl.start zygote")

}
