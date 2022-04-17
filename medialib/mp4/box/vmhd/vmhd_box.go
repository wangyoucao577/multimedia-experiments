// Package vmhd represents vmhd type box.
package vmhd

import (
	"encoding/binary"
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// Box represents a vmhd box.
type Box struct {
	box.FullHeader

	GraphicsMode uint16
	OPColor      [3]uint16
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
	return fmt.Sprintf("FullHeader:{%v} GraphicsMode:{%d} OPColor:{%v}", b.FullHeader, b.GraphicsMode, b.OPColor)
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

	var parsedBytes uint64
	data := make([]byte, 2)
	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		b.GraphicsMode = binary.BigEndian.Uint16(data)
		parsedBytes += 2
	}

	for i := 0; i < len(b.OPColor); i++ {
		if err := util.ReadOrError(r, data); err != nil {
			return err
		} else {
			b.OPColor[i] = binary.BigEndian.Uint16(data)
			parsedBytes += 2
		}
	}

	if parsedBytes != b.PayloadSize() {
		return fmt.Errorf("box %s parsed bytes != payload size: %d != %d", b.Type, parsedBytes, b.PayloadSize())
	}

	return nil
}
