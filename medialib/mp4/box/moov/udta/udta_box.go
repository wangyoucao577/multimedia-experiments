// Package udta represents user data boxes which may has type `udta`.
package udta

import (
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/moov/udta/cprt"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/moov/udta/meta"
)

// Box represents a udta box.
type Box struct {
	box.Header

	Cprt *cprt.Box
	Meta *meta.Box

	boxesCreator map[string]box.NewFunc
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		Header: h,

		boxesCreator: map[string]box.NewFunc{
			box.TypeCprt: cprt.New,
			box.TypeMeta: meta.New,
		},
	}
}

// CreateSubBox tries to create sub level box.
func (b *Box) CreateSubBox(h box.Header) (box.Box, error) {
	creator, ok := b.boxesCreator[h.Type.String()]
	if !ok {
		glog.V(2).Infof("unknown box type %s, size %d payload %d", h.Type.String(), h.Size, h.PayloadSize())
		return nil, box.ErrUnknownBoxType
	}

	createdBox := creator(h)
	if createdBox == nil {
		glog.Fatalf("create box type %s failed", h.Type.String())
	}

	if h.Type.String() == box.TypeCprt {
		b.Cprt = createdBox.(*cprt.Box)
	} else if h.Type.String() == box.TypeMeta {
		b.Meta = createdBox.(*meta.Box)
	}
	//TODO: other types

	return createdBox, nil
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Header:{%v} Cprt:{%v} Meta:{%v}", b.Header, b.Cprt, b.Meta)
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {
	if b.PayloadSize() == 0 {
		glog.Warningf("box %s is empty", b.Type)
		return nil
	}

	var parsedBytes uint64
	for {
		boxHeader, err := box.ParseBox(r, b)
		if err != nil {
			if err == io.EOF {
				return err
			} else if err == box.ErrUnknownBoxType {
				// after ignore the box, continue to parse next
			} else {
				return err
			}
		}
		parsedBytes += boxHeader.BoxSize()

		if parsedBytes == b.PayloadSize() {
			break
		}
	}

	return nil
}
