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

// SplitRelation ...
func SplitRelation(relation *Relation, svr string) (*Relation, error) {
	if relation == nil || len(svr) == 0 {
		return nil, errors.New("invalid parameters")
	}

	if _, ok := relation.M[svr]; !ok {
		return nil, errors.New("svr not found")
	}

	r := &Relation{make(map[string]map[string]int)}
	set := make(map[string]int)

	addRelation(svr, relation, r, set)

	return r, nil
}

func addRelation(svr string, relation *Relation, subset *Relation, set map[string]int) {
	set[svr] = 1
	m, ok := relation.M[svr]
	if !ok || len(m) == 0 {
		return
	}
	subset.M[svr] = m
	for s := range m {
		addRelation(s, relation, subset, set)
	}
}
