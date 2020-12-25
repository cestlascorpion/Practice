package awesome

import (
	"testing"
)

func TestAdd(t *testing.T) {
	if ans := Add(1, 2); ans != 3 {
		t.Errorf("1 + 2 expected be 3, but %d got", ans)
	}
	if ans := Add(-10, -20); ans != -30 {
		t.Errorf("-10 + -20 expected be -30, but %d got", ans)
	}
}

func TestSub(t *testing.T) {
	if ans := Sub(1, 2); ans != -1 {
		t.Errorf("1 + 2 expected be -1, but %d got", ans)
	}
	if ans := Sub(-10, -20); ans != 10 {
		t.Errorf("-10 + -20 expected be 10, but %d got", ans)
	}
}

func BenchmarkAdd(b *testing.B) {
	for i := 0; i < b.N; i++ {
		Add(i, i)
	}
}

func BenchmarkSub(b *testing.B) {
	for i := 0; i < b.N; i++ {
		Sub(i, i)
	}
}
