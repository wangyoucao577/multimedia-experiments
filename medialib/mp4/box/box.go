// Package box defines generic box structure.
package box

import (
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// FixedArray4Bytes represents 4 bytes array, mostly used for box type.
type FixedArray4Bytes [4]byte

// String serializes FixedArray4Bytes.
func (f FixedArray4Bytes) String() string {
	return string(f[:])
}

// MarshalJSON implements json.Marshaler interface.
func (f FixedArray4Bytes) MarshalJSON() ([]byte, error) {
	return json.Marshal(f.String())
}

// Header represents box header structure.
type Header struct {
	Size      uint32           `json:"size"`
	Type      FixedArray4Bytes `json:"type"` // 32 bits
	LargeSize uint64           `json:"large_size,omitempty"`
	UserType  *[16]uint8       `json:"user_type,omitempty"`

	// internal fields
	payloadSize uint64 `json:"-"` // includes full box additional version and flags if exist
}

// FullHeader represents full box header structure.
type FullHeader struct {
	Header `json:"header"`

	Version uint8  `json:"version"`
	Flags   uint32 `json:"flags"` // 24bits
}

// String serializes Header.
func (h Header) String() string {
	return fmt.Sprintf("Size:%d Type:%s LargeSize:%d UserType:%s payloadSize:%d", h.Size, h.Type[:], h.LargeSize, (h.UserType[:]), h.payloadSize)
}

// PayloadSize returns payload size, 0 means continue to the end.
func (h Header) PayloadSize() uint64 {
	return h.payloadSize
}

// Size returns total box bytes.
func (h Header) BoxSize() uint64 {
	if h.Size == 1 {
		return h.LargeSize
	}
	return uint64(h.Size)
}

// Parse parses basic box contents.
func (h *Header) Parse(r io.Reader) error {
	var parsedBytes uint32

	data := make([]byte, 4)

	// size
	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		h.Size = binary.BigEndian.Uint32(data)
		parsedBytes += 4
	}

	// type
	if err := util.ReadOrError(r, h.Type[:]); err != nil {
		return err
	}
	parsedBytes += 4

	// large size
	if h.Size == 1 {
		largeData := make([]byte, 8)

		if err := util.ReadOrError(r, largeData); err != nil {
			return err
		} else {
			h.LargeSize = binary.BigEndian.Uint64(largeData)
			parsedBytes += 8
		}
	}

	// user type
	if string(h.Type[:]) == TypeUUID {
		h.UserType = &[16]uint8{}
		if err := util.ReadOrError(r, h.UserType[:]); err != nil {
			return err
		}
		parsedBytes += 16
	}

	if h.Size == 1 {
		h.payloadSize = uint64(h.LargeSize) - uint64(parsedBytes)
	} else if h.Size > 1 {
		h.payloadSize = uint64(h.Size) - uint64(parsedBytes)
	}

	return nil
}

// Validate validates box header.
func (h *Header) Validate() error {
	if !IsValidBoxType(string(h.Type[:])) {
		return fmt.Errorf("invalid box type %s", h.Type)
	}
	return nil
}

// String serializes Header.
func (h FullHeader) String() string {
	return fmt.Sprintf("Header:{%v} Version:%d Flags:%x",
		h.Header, h.Version, h.Flags)
}

// ParseVersionFlag assumes Header has been prepared already,
// 	and try to parse additional `version` and `flag`.`
// Be aware that it will decrease `PayloadSize` after succeed.
func (f *FullHeader) ParseVersionFlag(r io.Reader) error {
	if err := f.Header.Validate(); err != nil {
		return err
	}

	data := make([]byte, 4)
	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		f.Version = data[0]
		f.Flags = binary.BigEndian.Uint32(data)
	}

	// minus used bytes for accurate payload size
	f.payloadSize -= 4
	return nil
}
