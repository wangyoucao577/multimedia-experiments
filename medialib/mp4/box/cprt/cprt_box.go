// Package cprt represents copy right boxes which may has type `cprt`.
package cprt

import (
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// Box represents a cprt box.
type Box struct {
	box.FullHeader

	Pad      uint8    // 1 bit
	Language [3]uint8 // 5 bytes per uint
	Notice   string
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		FullHeader: box.FullHeader{
			Header: h,
		},
	}
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("FullHeader:{%v}", b.FullHeader)
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {
	if b.PayloadSize() == 0 {
		glog.Warningf("box %s is empty", b.Type)
		return nil
	}

	// parse full header additional information first
	if err := b.FullHeader.ParseVersionFlag(r); err != nil {
		return err
	}

	// start to parse payload
	var parsedBytes uint64

	data := make([]byte, 2)
	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		b.Pad = (uint8(data[0]) >> 7) & 0x1 // 1 bit

		// 5 bits per Language
		b.Language[0] = (uint8(data[0]) >> 2) & 0x1F
		b.Language[1] = ((uint8(data[0]) & 0x3) << 3) | ((uint8(data[1]) >> 5) & 0x7)
		b.Language[2] = uint8(data[1]) & 0x1F

		parsedBytes += 2
	}

	data = make([]byte, b.PayloadSize()-parsedBytes)
	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		b.Notice = string(data)
	}

	return nil
}
