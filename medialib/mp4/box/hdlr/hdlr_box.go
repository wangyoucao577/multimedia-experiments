// Package hdlr represents hdlr type box.
package hdlr

import (
	"encoding/binary"
	"fmt"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// Box represents a hdlr box.
type Box struct {
	box.FullHeader `json:"full_header"`

	Predefined  uint32               `json:"pre_defined"`
	HandlerType box.FixedArray4Bytes `json:"handler_type"`
	// reserved 32 * 3 = 96 bits
	Name string `json:"name"`
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		FullHeader: box.FullHeader{
			Header: h,
		},
	}
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {
	if b.PayloadSize() == 0 {
		return fmt.Errorf("box %s is empty", b.Type)
	}

	// parse full header additional information first
	if err := b.FullHeader.ParseVersionFlag(r); err != nil {
		return err
	}

	var parsedBytes uint64
	data := make([]byte, 4)

	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		b.Predefined = binary.BigEndian.Uint32(data)
		parsedBytes += 4
	}

	if err := util.ReadOrError(r, b.HandlerType[:]); err != nil {
		return err
	} else {
		parsedBytes += 4
	}

	// reserved bytes
	if err := util.ReadOrError(r, make([]byte, 12)); err != nil {
		return err
	} else {
		parsedBytes += 12
	}

	nameBytes := int64(b.PayloadSize()) - int64(parsedBytes)
	if nameBytes < 0 {
		return fmt.Errorf("box %s error remain data %d", b.Type, nameBytes)
	}
	if nameBytes > 0 {
		data = make([]byte, nameBytes)
		if err := util.ReadOrError(r, data); err != nil {
			return err
		}
		b.Name = string(data)
	}

	return nil
}
