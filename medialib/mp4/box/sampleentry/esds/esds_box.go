// Package esds represents ES Descriptor Box.
package esds

import (
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// Box represents a esds box.
type Box struct {
	box.FullHeader `json:"full_header"`

	ESDescriptor `json:"es_descriptor"`
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
	if err := b.Validate(); err != nil {
		glog.Warningf("box %s invalid, err %v", b.Type, err)
		return nil
	}

	// parse full header additional information first
	if err := b.FullHeader.ParseVersionFlag(r); err != nil {
		return err
	}

	var parsedBytes uint64

	if bytes, err := b.ESDescriptor.parse(r); err != nil {
		return err
	} else {
		parsedBytes += bytes
	}

	if b.PayloadSize() > parsedBytes {
		glog.Warningf("box type %s remain bytes %d parsing TODO", b.Type, b.PayloadSize()-parsedBytes)
		//TODO: parse payload
		if err := util.ReadOrError(r, make([]byte, b.PayloadSize()-parsedBytes)); err != nil {
			return err
		}
	}

	return nil
}
