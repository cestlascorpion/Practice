package music

import (
	"errors"
)

// Library 音乐库
type Library struct {
	list []Music
}

// NewLibrary 创建音乐库
func NewLibrary() *Library {
	return &Library{
		list: make([]Music, 0),
	}
}

// Len 查询音乐库数量
func (l *Library) Len() int {
	return len(l.list)
}

// Get 获取音乐文件
func (l *Library) Get(index int) (*Music, error) {
	if index < 0 || index >= len(l.list) {
		return nil, errors.New("Index out of range")
	}
	return &l.list[index], nil
}

// Find 查找音乐文件
func (l *Library) Find(name string) *Music {
	if len(l.list) == 0 {
		return nil
	}
	for _, m := range l.list {
		if m.Name == name {
			return &m
		}
	}
	return nil
}

// Add 添加音乐文件
func (l *Library) Add(music *Music) {
	l.list = append(l.list, *music)
}

// Remove 删除音乐文件
func (l *Library) Remove(index int) *Music {
	if index < 0 || index >= len(l.list) {
		return nil
	}

	removed := &l.list[index]
	l.list = append(l.list[:index], l.list[index+1:]...)
	return removed
}
