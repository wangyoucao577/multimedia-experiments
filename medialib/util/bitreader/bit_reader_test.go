package bitreader

import (
	"bytes"
	"testing"
)

func TestReadBit(t *testing.T) {
	cases := []struct {
		count int
		in    []byte
		out   []byte
	}{
		{count: 8, in: []byte{0xAA}, out: []byte{0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0}},
		{count: 8, in: []byte{0x55}, out: []byte{0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1}},
		{count: 10, in: []byte{0x55, 0xFF}, out: []byte{0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x1, 0x1}},
	}

	for _, c := range cases {
		r := bytes.NewReader(c.in)
		br := New(r)

		for i := 0; i < c.count; i++ {
			if nextBit, err := br.ReadBit(); err != nil {
				t.Error(err)
			} else if nextBit != c.out[i] {
				t.Errorf("read bit %d] expect 0x%x but got 0x%x", i, c.out[i], nextBit)
			}
		}
	}
}
