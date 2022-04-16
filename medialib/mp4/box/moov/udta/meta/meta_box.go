// Package meta represents copy right boxes which may has type `meta`.
package meta

import (
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
)

// Box represents a meta box.
type Box struct {
	box.FullHeader
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

	glog.Warningf("box type %s payload bytes %d parsing TODO", b.Type, b.PayloadSize())
	//TODO: parse payload
	if _, e := r.Read(make([]byte, b.PayloadSize())); e != nil {
		return fmt.Errorf("parse box %s read %d bytes failed, err %v", b.Type, b.PayloadSize(), e)
	}

	return nil
}