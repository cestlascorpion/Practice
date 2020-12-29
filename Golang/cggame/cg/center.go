package cg

import (
	"encoding/json"
	"golang/cggame/ipc"
	"sync"
)

// Message ...
type Message struct {
	From    string `json:"from"`
	To      string `json:"to"`
	Content string `json:"content"`
}

// CenterServer ...
type CenterServer struct {
	servers map[string]ipc.MyServer
	players []*Player
	rooms   []*Room
	mutex   sync.RWMutex
}

// NewCenterServer ...
func NewCenterServer() *CenterServer {
	return &CenterServer{
		servers: make(map[string]ipc.MyServer),
		players: make([]*Player, 0),
		rooms:   make([]*Room, 0),
	}
}

func (s *CenterServer) addPlayer(params string) error {
	player := NewPlayer()
	err := json.Unmarshal([]byte(params), &player)
	if err != nil {
		return err
	}
	s.mutex.Lock()
	defer s.mutex.Unlock()
	s.players = append(s.players, player)
	return nil
}

func (s *CenterServer) removePlayer(name string) error {
	s.mutex.Lock()
	defer s.mutex.Unlock()
	for i, v := range s.players {
		if v.Name == name {
			if len(s.players) == 1 {
				s.players = make([]*Player, 0)
			} else if i == len(s.players)-1 {
				s.players = s.players[:i]
			} else if i == 0 {
				s.players = s.players[1:]
			} else {
				s.players = append(s.players[:i], s.players[i+1:]...)
			}
		}
	}
	return nil
}
