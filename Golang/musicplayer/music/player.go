package music

import (
	"fmt"
	"time"
)

// Play 播放音乐
func Play(source, kind string) {
	var p player
	switch kind {
	case "MP3":
		p = &mp3Player{0}
	case "WAV":
		p = &wavPlayer{0}
	default:
		fmt.Println("Unknown music type")
		return
	}
	p.play(source)
}

// Player 播放器
type player interface {
	play(source string)
}

type mp3Player struct {
	progress int
}

func (p *mp3Player) play(source string) {
	fmt.Println("play begin", source)
	for p.progress < 10 {
		fmt.Print(".")
		time.Sleep(time.Second)
		p.progress++
	}
	fmt.Println()
	fmt.Println("play end", source)
}

type wavPlayer struct {
	progress int
}

func (p *wavPlayer) play(source string) {
	fmt.Println("play begin", source)
	for p.progress < 10 {
		fmt.Print("*")
		time.Sleep(time.Second)
		p.progress++
	}
	fmt.Println()
	fmt.Println("play end", source)
}
