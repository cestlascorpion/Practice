package cg

import (
	"testing"
	"time"
)

func TestNewPlayer(t *testing.T) {
	player := NewPlayer()
	if player == nil {
		t.Fatal("new player is nil")
	}
	player.Name = "test"
	player.mq <- &Message{
		From:    "one",
		To:      "two",
		Content: "Test",
	}
	close(player.mq) // not necessary
	time.Sleep(time.Millisecond * 100)
}
