package helper

import (
	"testing"
)

const (
	testSvr = "accountlogic"
)

func TestGenerateSvrGraph(t *testing.T) {
	re, err := ParseRelation(relation)
	if err != nil {
		t.Fatal(err)
	}

	err = GenerateSvrGraph(testSvr, re, true)
	if err != nil {
		t.Fatal(testSvr, err)
	}

	err = GenerateSvrGraph(testSvr, re, false)
	if err != nil {
		t.Fatal(testSvr, err)
	}
}
