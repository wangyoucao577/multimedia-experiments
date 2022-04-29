package expgolombcoding

import (
	"encoding/json"
	"io"
)

// Signed contains signed Exponential-Golomb coded integer.
type Signed struct {
	unsigned Unsigned
}

// MarshalJSON implements json.Marshaler.
func (s *Signed) MarshalJSON() ([]byte, error) {
	return json.Marshal(s.Value())
}

// Value returns parsed value.
func (s *Signed) Value() int64 {
	//TODO: correct signed value
	return int64(s.unsigned.value)
}

// SetRemainBits sets bits for parsing.
//   The bits should have (8-numOfBits) padding zero bits at the beginning of the byte, which should be ignored.
//   For example, bits = [0, 0, 0, 0, 1, 0, 0, 0], numOfBit=5, then the first 3 `0` should be ignored.
//   It should be called before `Parse` if need.
func (s *Signed) SetRemainBits(prefixBits byte, numOfBits int) {
	s.unsigned.SetRemainBits(prefixBits, numOfBits)
}

// RemainBits return remain bits and num of bits.
func (s *Signed) RemainBits() (byte, int) {
	return s.unsigned.RemainBits()
}

// Parse parses unsigned value of Exponential-Golomb Coding.
// return the cost bits(NOT Byte) if succeed, otherwise error.
func (s *Signed) Parse(r io.Reader) (uint64, error) {
	return s.unsigned.Parse(r)
}
