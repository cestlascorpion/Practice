package helper

import (
	"testing"
)

func TestParseCppDBRelation(t *testing.T) {
	r, err := ParseDBRelation(CPP, KAFKA, PREFIX+CPP+KAFKA+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}
	r.Print()

	r, err = ParseDBRelation(CPP, MONGO, PREFIX+CPP+MONGO+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}
	r.Print()

	r, err = ParseDBRelation(CPP, MYSQL, PREFIX+CPP+MYSQL+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}
	r.Print()

	r, err = ParseDBRelation(CPP, REDIS, PREFIX+CPP+REDIS+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}
	r.Print()

	r, err = ParseDBRelation(CPP, TIDB, PREFIX+CPP+TIDB+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}
	r.Print()
}

func TestParseGolangDBRelation(t *testing.T) {
	r, err := ParseDBRelation(GOLANG, KAFKA, PREFIX+GOLANG+KAFKA+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}
	r.Print()

	r, err = ParseDBRelation(GOLANG, MONGO, PREFIX+GOLANG+MONGO+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}
	r.Print()

	r, err = ParseDBRelation(GOLANG, MYSQL, PREFIX+GOLANG+MYSQL+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}
	r.Print()

	r, err = ParseDBRelation(GOLANG, REDIS, PREFIX+GOLANG+REDIS+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}
	r.Print()

	r, err = ParseDBRelation(GOLANG, TIDB, PREFIX+GOLANG+TIDB+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}
	r.Print()
}
