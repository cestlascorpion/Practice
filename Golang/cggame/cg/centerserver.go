package cg

import (
	"encoding/json"
	"errors"
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
	mutex   sync.RWMutex
}

// NewCenterServer ...
func NewCenterServer() *CenterServer {
	return &CenterServer{
		servers: make(map[string]ipc.MyServer),
		players: make([]*Player, 0),
	}
}

// Name ... implement Interface Server
func (s *CenterServer) Name() string {
	return "CenterServer"
}

// Handle ... implement Interface Server
func (s *CenterServer) Handle(method, params string) *ipc.Response {
	switch method {
	case "addPlayer":
		err := s.addPlayer(params)
		if err != nil {
			return &ipc.Response{Code: "Err", Body: err.Error()}
		}
		return &ipc.Response{Code: "Ok"}
	case "removePlayer":
		err := s.removePlayer(params)
		if err != nil {
			return &ipc.Response{Code: "Err", Body: err.Error()}
		}
		return &ipc.Response{Code: "Ok"}
	case "listPlayer":
		list, err := s.listPlayer(params)
		if err != nil {
			return &ipc.Response{Code: "Err", Body: err.Error()}
		}
		return &ipc.Response{Code: "Ok", Body: list}
	case "broadCast":
		err := s.broadCast(params)
		if err != nil {
			return &ipc.Response{Code: "Err", Body: err.Error()}
		}
		return &ipc.Response{Code: "Ok", Body: ""}
	default:
		return &ipc.Response{Code: "Err", Body: "unknown method: " + method + " - " + params}
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

func (s *CenterServer) listPlayer(string) (string, error) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	if len(s.players) > 0 {
		list, _ := json.Marshal(s.players)
		return string(list), nil
	}
	return "", errors.New("no player online")
}

func (s *CenterServer) broadCast(params string) error {
	var message Message
	err := json.Unmarshal([]byte(params), &message)
	if err != nil {
		return err
	}

	s.mutex.Lock()
	defer s.mutex.Unlock()

	if len(s.players) > 0 {
		for _, player := range s.players {
			player.mq <- &message
		}
		return nil
	}

	return errors.New("no player online")
}
