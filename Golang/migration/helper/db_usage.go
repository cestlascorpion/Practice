package helper

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"strings"
)

// Print ...
func (d *DBRelation) Print() {
	fmt.Println(d.L, d.D)
	for key, val := range d.M {
		fmt.Print(key, ":")
		for db := range val {
			fmt.Print(" ", db)
		}
		fmt.Println()
	}
}

// ParseDBRelation ...
func ParseDBRelation(language, database, file string) (*DBRelation, error) {
	if len(language) == 0 || len(database) == 0 || len(file) == 0 {
		return nil, errors.New("invalid parameters")
	}
	f, err := os.Open(file)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	dr := &DBRelation{language, database, make(map[string]map[string]int, 0)}
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := scanner.Text()
		if len(line) == 0 {
			continue
		}
		s := strings.Split(line, " ")
		if len(s) < 2 {
			return nil, errors.New("bad format")
		}
		_, ok := dr.M[s[0]]
		if !ok {
			dr.M[s[0]] = make(map[string]int, 0)
		}
		for i, v := range s {
			if i == 0 {
				continue
			}
			dr.M[s[0]][v] = 1
		}
	}
	return dr, nil
}

// GenerateDBUsages ...
func GenerateDBUsages(language string) ([]*DBRelation, error) {
	drs := make([]*DBRelation, 0)

	r1, err := ParseDBRelation(language, KAFKA, PREFIX+language+KAFKA+SUFFIX)
	if err != nil {
		return nil, err
	}
	drs = append(drs, r1)

	r2, err := ParseDBRelation(language, MONGO, PREFIX+language+MONGO+SUFFIX)
	if err != nil {
		return nil, err
	}
	drs = append(drs, r2)

	r3, err := ParseDBRelation(language, MYSQL, PREFIX+language+MYSQL+SUFFIX)
	if err != nil {
		return nil, err
	}
	drs = append(drs, r3)

	r4, err := ParseDBRelation(language, REDIS, PREFIX+language+REDIS+SUFFIX)
	if err != nil {
		return nil, err
	}
	drs = append(drs, r4)

	r5, err := ParseDBRelation(language, TIDB, PREFIX+language+TIDB+SUFFIX)
	if err != nil {
		return nil, err
	}
	drs = append(drs, r5)

	return drs, nil
}
