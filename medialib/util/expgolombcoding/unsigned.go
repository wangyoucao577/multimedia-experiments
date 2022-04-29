// Package expgolombcoding implements Exponential-Golomb Coding,
// see more in https://en.wikipedia.org/wiki/Exponential-Golomb_coding or ISO/IEC-14496-10 7.2
package expgolombcoding

import (
	"encoding/json"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

const bitsPerByte = 8

// Unsigned represents unsigned Exponential-Golomb coding.
type Unsigned struct {

	// remain bits for following parsing, 0~8 bits.
	// It will be set either before Unsigned parse(pass bits in) or after Unsigned parse(pass bits out for other parsing).
	remainBits      byte
	numOfRemainBits int

	// parsing
	leadingZeroBit int
	parsedBits     int

	// store parsed
	value uint64
}

// MarshalJSON implements json.Marshaler.
func (u Unsigned) MarshalJSON() ([]byte, error) {
	return json.Marshal(u.Value())
}

// Value returns parsed value.
func (u *Unsigned) Value() uint64 {
	return u.value
}

// SetRemainBits sets bits for parsing.
//   The bits should have (8-numOfBits) padding zero bits at the beginning of the byte, which should be ignored.
//   For example, bits = [0, 0, 0, 0, 1, 0, 0, 0], numOfBit=5, then the first 3 `0` should be ignored.
//   It should be called before `Parse` if need.
func (u *Unsigned) SetRemainBits(bits byte, numOfBits int) {
	if numOfBits < 0 || numOfBits > bitsPerByte {
		glog.Warningf("invalid numOfBits %d, ignore it", numOfBits)
		return
	}

	u.remainBits = bits
	u.numOfRemainBits = numOfBits
}

// RemainBits return remain bits and num of bits.
func (u *Unsigned) RemainBits() (byte, int) {
	return u.remainBits, u.numOfRemainBits
}

// Parse parses unsigned value of Exponential-Golomb Coding.
// return the cost bits(NOT Byte) if succeed, otherwise error.
func (u *Unsigned) Parse(r io.Reader) (uint64, error) {

	u.leadingZeroBit = -1
	for b := uint8(0); b == 0; u.leadingZeroBit++ {
		var err error
		if b, err = u.readBit(r); err != nil {
			return uint64(u.parsedBits), err
		}
	}

	for i := 0; i < u.leadingZeroBit; i++ {
		if b, err := u.readBit(r); err != nil {
			return uint64(u.parsedBits), err
		} else {
			u.value = (u.value << 1) | uint64(b)
		}
	}

	u.value += (1 << u.leadingZeroBit) - 1
	return uint64(u.parsedBits), nil
}

func (u *Unsigned) readBit(r io.Reader) (byte, error) {

	if u.numOfRemainBits <= 0 {
		nextByte := make([]byte, 1)
		if err := util.ReadOrError(r, nextByte); err != nil {
			return 0, err
		} else {
			u.remainBits = nextByte[0]
			u.numOfRemainBits = bitsPerByte
		}
	}

	u.parsedBits += 1
	u.numOfRemainBits -= 1
	return (u.remainBits >> u.numOfRemainBits) & 0x1, nil
}
