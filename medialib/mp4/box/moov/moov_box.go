// Package moov defines moov box structure which contains meta data.
package moov

import (
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// Box represents a mdat box.
type Box struct {
	box.Header

	//TODO:
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

	payloadSize := b.PayloadSize()

	if payloadSize <= 0 {
		glog.Warning("no payload in mdat box")
		return nil
	}

	//TODO:
	glog.Warning("TODO: moov box payload parse")
	data := make([]byte, payloadSize)
	if err := util.ReadOrError(r, data); err != nil {
		return err
	}

	return nil
}
