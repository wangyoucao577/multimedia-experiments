// Package mdat defines mdat box structure which contains media data.
package mdat

import (
	"fmt"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// Box represents a mdat box.
type Box struct {
	box.Header

	Data []byte
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		Header: h,
	}
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Header:{%v} data size:%d",
		b.Header, len(b.Data))
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {

	payloadSize := b.PayloadSize()

	if payloadSize == 0 {
		return fmt.Errorf("box %s is empty", b.Type)
	}

	b.Data = make([]byte, payloadSize)
	if err := util.ReadOrError(r, b.Data); err != nil {
		return err
	}

	return nil
}
