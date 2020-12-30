package cg

import "fmt"

// Player ...
type Player struct {
	Name  string
	Level int
	Exp   int
	Room  int
	mq    chan *Message
}

// NewPlayer ...
func NewPlayer() *Player {
	m := make(chan *Message, 1024)
	player := &Player{"", 0, 0, 0, m}

	go func(p *Player) {
		for {
			msg, ok := <-p.mq
			if !ok {
				fmt.Println("player", p.Name, "mq closed")
				return
			}
			if len(msg.From) > 0 && len(msg.To) > 0 {
				fmt.Println(p.Name, "recv p2p msg", msg.From, "->", msg.To, ":", msg.Content)
			} else {
				fmt.Println(p.Name, "recv broadcast msg:", msg.Content)
			}

		}
	}(player)

	return player
}
