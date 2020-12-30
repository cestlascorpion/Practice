package main

import (
	"bufio"
	"fmt"
	"golang/cggame/cg"
	"golang/cggame/ipc"
	"os"
	"strconv"
	"strings"
)

var centerClient *cg.CenterClient

func startCenterService() {
	server := ipc.NewMyServer(cg.NewCenterServer())
	centerClient = cg.NewCenterClient(server)
}

func Help(args []string) int {
	fmt.Print("Commands:\n\t\tlogin <name> <level> <exp>\n\t\tlogout <name>\n\t\tsend <message>\n\t\tlistPlayer\n\t\tquit(q)\n\t\thelp(h)\n")
	return 0
}

func Quit(args []string) int {
	return 1
}

func Logout(args []string) int {
	if len(args) != 2 {
		fmt.Println("USAGE: logout <name>")
		return 0
	}
	err := centerClient.RemovePlayer(args[1])
	if err != nil {
		fmt.Println(err)
	}
	return 0
}

func Login(args []string) int {
	if len(args) != 4 {
		fmt.Println("USAGE: login <name> <level> <exp>")
		return 0
	}

	level, err := strconv.Atoi(args[2])
	if err != nil {
		fmt.Println("invalid level", args[2])
		return 0
	}

	exp, err := strconv.Atoi(args[3])
	if err != nil {
		fmt.Println("invalid exp", args[3])
		return 0
	}

	player := cg.NewPlayer()
	player.Name = args[1]
	player.Level = level
	player.Exp = exp

	e1 := centerClient.AddPlayer(player)
	if e1 != nil {
		fmt.Println(err)
	}
	return 0
}

func ListPlayer(args []string) int {
	list, err := centerClient.ListPlayer("")
	if err != nil {
		fmt.Println(err)
	} else {
		for i, v := range list {
			fmt.Println(i+1, ":", v)
		}
	}
	return 0
}

func Send(args []string) int {
	m := strings.Join(args[1:], " ")
	err := centerClient.BroadCast(m)
	if err != nil {
		fmt.Println(err)
	}
	return 0
}

func GetCommandHandlers() map[string]func([]string) int {
	return map[string]func([]string) int{
		"help":       Help,
		"h":          Help,
		"quit":       Quit,
		"q":          Quit,
		"login":      Login,
		"logout":     Logout,
		"send":       Send,
		"listPlayer": ListPlayer,
	}
}

func main() {
	startCenterService()
	Help(nil)
	r := bufio.NewReader(os.Stdin)
	handlers := GetCommandHandlers()

	for {
		fmt.Println("Command>")
		b, _, _ := r.ReadLine()
		line := string(b)
		tokens := strings.Split(line, " ")

		if handler, ok := handlers[tokens[0]]; ok {
			ret := handler(tokens)
			if ret != 0 {
				break
			}
		} else {
			fmt.Println("unknown command")
		}
	}
}
