package helper

import (
	"testing"
)

func TestParseDBRelation(t *testing.T) {
	re, err := ParseRelation(relation)
	if err != nil {
		t.Fatal(err)
	}

	cppKafka, err := ParseDBRelation(CPP, MONGO, PREFIX+CPP+KAFKA+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}

	cppMongo, err := ParseDBRelation(CPP, MONGO, PREFIX+CPP+MONGO+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}

	cppMySQL, err := ParseDBRelation(CPP, MYSQL, PREFIX+CPP+MYSQL+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}

	cppRedis, err := ParseDBRelation(CPP, REDIS, PREFIX+CPP+REDIS+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}

	cppTiDB, err := ParseDBRelation(CPP, TIDB, PREFIX+CPP+TIDB+SUFFIX)
	if err != nil {
		t.Fatal(err)
	}

	err = GenerateCppDBGraph(testSvr, re, []*DBRelation{cppKafka, cppMongo, cppMySQL, cppRedis, cppTiDB})
	if err != nil {
		t.Fatal(testSvr, err)
	}
}
