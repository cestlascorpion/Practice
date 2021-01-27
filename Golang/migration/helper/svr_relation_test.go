package helper

import (
	"testing"
)

const (
	relation = "/home/godman/Workspace/Practice/Golang/migration/conf/relation.txt"
)

func TestParseRelation(t *testing.T) {
	r, err := ParseRelation(relation)
	if err != nil {
		t.Fatal(err)
	}
	r.Print()
}
