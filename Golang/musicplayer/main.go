package main

import (
	"fmt"
	"golang/musicplayer/music"
)

func main() {
	library := music.NewLibrary()

	library.Add(&music.Music{
		Name:   "Shallow",
		Artist: "LadyGaga",
		Type:   "MP3",
	})
	library.Add(&music.Music{
		Name:   "Style",
		Artist: "TaylorSwift",
		Type:   "MP3",
	})
	library.Add(&music.Music{
		Name:   "BlindingLights",
		Artist: "TheWeeknd",
		Type:   "WAV",
	})
	println("library size:", library.Len())

	m := library.Find("Style")
	if m != nil {
		music.Play(m.Name, m.Type)
	}

	{
		m, err := library.Get(3)
		if err != nil {
			fmt.Println(err)
		} else {
			music.Play(m.Name, m.Type)
		}
	}

	{
		m, err := library.Get(2)
		if err != nil {
			fmt.Println(err)
		} else {
			music.Play(m.Name, m.Type)
		}
	}

}
