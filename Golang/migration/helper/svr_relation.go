package helper

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"strings"
)

// Print ...
func (r *Relation) Print() {
	for key, val := range r.M {
		fmt.Print(key, ":")
		for svr := range val {
			fmt.Print(" ", svr)
		}
		fmt.Println()
	}
}

// ParseRelation ...
func ParseRelation(file string) (*Relation, error) {
	if len(file) == 0 {
		return nil, errors.New("invalid file")
	}
	f, err := os.Open(file)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	r := &Relation{make(map[string]map[string]int, 0)}
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := scanner.Text()
		if len(line) == 0 {
			continue
		}
		s := strings.Fields(line)
		if s == nil || len(s) < 2 {
			return nil, errors.New("bad format")
		}

		_, ok := r.M[s[0]]
		if !ok {
			r.M[s[0]] = make(map[string]int, 0)
		}
		for i, v := range s {
			if i == 0 {
				continue
			}
			if v == s[0] {
				continue
			}
			r.M[s[0]][v] = 1
		}
	}
	return r, nil
}
