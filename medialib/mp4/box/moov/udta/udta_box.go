// Package udta represents user data boxes which may has type `udta`.
package udta

import (
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// Box represents a ftyp box.
type Box struct {
	box.Header
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		Header: h,
	}
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Header:{%v}", b.Header)
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {
	if b.PayloadSize() == 0 {
		glog.Warningf("box %s is empty", b.Type)
		return nil
	}

	// TODO:
	glog.Warningf("box type %s ignore bytes %d", b.Type, b.PayloadSize())
	data := make([]byte, b.PayloadSize())
	if err := util.ReadOrError(r, data); err != nil {
		return err
	}

	return nil
}
