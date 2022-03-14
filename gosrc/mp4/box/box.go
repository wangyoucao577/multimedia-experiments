// Package box defines generic box structure.
package box

import (
	"encoding/binary"
	"fmt"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/gosrc/util"
)

// Box types
const (
	TypeFtyp = "ftyp"
	TypeFree = "free"

	TypeUUID = "uuid"
)

// FixedArray4Bytes represents 4 bytes array.
type FixedArray4Bytes [4]byte

// Header represents box header structure.
type Header struct {
	Size      uint32
	Type      FixedArray4Bytes // 32 bits
	LargeSize uint64
	UserType  [16]uint8
}

// Box represents box structure.
type Box struct {
	Header

	Version uint8
	Flags   [3]byte // 24 bits

	// internal fields
	payloadSize uint64
}

// String serializes FixedArray4Bytes.
func (f FixedArray4Bytes) String() string {
	return string(f[:])
}

// String serializes Header.
func (h Header) String() string {
	return fmt.Sprintf("Size:%d Type:%s LargeSize:%d UserType:%s", h.Size, string(h.Type[:]), h.LargeSize, (h.UserType[:]))
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Header:{%+v} Version:%d Flags:%v PayloadSize:%d", b.Header, b.Version, b.Flags, b.payloadSize)
}

// PayloadSize returns payload size, 0 means continue to the end.
func (b *Box) PayloadSize() uint64 {
	return b.payloadSize
}

// Parse parses basic box contents.
func (b *Box) Parse(r io.Reader) error {
	var parsedBytes uint32

	data := make([]byte, 4)

	// size
	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		b.Size = binary.BigEndian.Uint32(data)
		parsedBytes += 4
	}

	// type
	if err := util.ReadOrError(r, b.Type[:]); err != nil {
		return err
	}
	parsedBytes += 4

	// large size
	if b.Size == 1 {
		largeData := make([]byte, 8)

		if err := util.ReadOrError(r, largeData); err != nil {
			return err
		} else {
			b.LargeSize = binary.BigEndian.Uint64(largeData)
			parsedBytes += 8
		}
	}

	// user type
	if string(b.Type[:]) == TypeUUID {
		if err := util.ReadOrError(r, b.UserType[:]); err != nil {
			return err
		}
		parsedBytes += 16
	}

	if b.Size == 0 || b.Size > parsedBytes || b.LargeSize > uint64(parsedBytes) {
		if err := util.ReadOrError(r, data); err != nil {
			return err
		} else {
			b.Version = data[0]
			copy(b.Flags[:], data[1:])
			parsedBytes += 4
		}
	}

	if b.Size == 1 {
		b.payloadSize = uint64(b.LargeSize) - uint64(parsedBytes)
	} else if b.Size > 1 {
		b.payloadSize = uint64(b.Size) - uint64(parsedBytes)
	}

	return nil
}
