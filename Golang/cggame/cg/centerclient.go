package cg

import (
	"encoding/json"
	"golang/cggame/ipc"
)

// CenterClient ...
type CenterClient struct {
	*ipc.MyClient
}

// NewCenterClient ...
func NewCenterClient(server *ipc.MyServer) *CenterClient {
	return &CenterClient{ipc.NewMyClient(server)}
}

// AddPlayer ...
func (c *CenterClient) AddPlayer(player *Player) error {
	b, err := json.Marshal(player)
	if err != nil {
		return err
	}

	resp, err := c.Call("addPlayer", string(b))
	if err == nil && resp.Code == "Ok" {
		return nil
	}
	return err
}

// RemovePlayer ...
func (c *CenterClient) RemovePlayer(name string) error {
	resp, err := c.Call("removePlayer", name)
	if err == nil && resp.Code == "Ok" {
		return nil
	}
	return err
}

// ListPlayer ...
func (c *CenterClient) ListPlayer(params string) ([]*Player, error) {
	resp, err := c.Call("listPlayer", params)
	if err == nil && resp.Code == "Ok" {
		list := make([]*Player, 0)
		err = json.Unmarshal([]byte(resp.Body), &list)
		return list, err
	}
	return nil, nil

}

// BroadCast ...
func (c *CenterClient) BroadCast(message string) error {
	m := Message{Content: message}
	b, err := json.Marshal(m)
	if err != nil {
		return err
	}

	resp, err := c.Call("broadCast", string(b))
	if err == nil && resp.Code == "Ok" {
		return nil
	}
	return err
}
