// Package dinf represents dinf type box.
package dinf

import (
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// Box represents a dinf box.
type Box struct {
	box.Header `json:"header"`
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		Header: h,
	}
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {
	if b.PayloadSize() == 0 {
		glog.Warningf("box %s is empty", b.Type)
		return nil
	}

	glog.Warningf("box type %s payload bytes %d parsing TODO", b.Type, b.PayloadSize())
	//TODO: parse payload
	if err := util.ReadOrError(r, make([]byte, b.PayloadSize())); err != nil {
		return err
	}

	return nil
}
