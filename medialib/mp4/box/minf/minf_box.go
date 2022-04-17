// Package minf represents minf type box.
package minf

import (
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/dinf"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/smhd"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/stbl"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/vmhd"
)

// Box represents a minf box.
type Box struct {
	box.Header

	Stbl *stbl.Box
	Dinf *dinf.Box
	Smhd *smhd.Box
	Vmhd *vmhd.Box

	boxesCreator map[string]box.NewFunc
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		Header: h,

		boxesCreator: map[string]box.NewFunc{
			box.TypeStbl: stbl.New,
			box.TypeDinf: dinf.New,
			box.TypeSmhd: smhd.New,
			box.TypeVmhd: vmhd.New,
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

	switch h.Type.String() {
	case box.TypeStbl:
		b.Stbl = createdBox.(*stbl.Box)
	case box.TypeDinf:
		b.Dinf = createdBox.(*dinf.Box)
	case box.TypeSmhd:
		b.Smhd = createdBox.(*smhd.Box)
	case box.TypeVmhd:
		b.Vmhd = createdBox.(*vmhd.Box)
	}

	return createdBox, nil
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Header:{%v} Sbtl:{%v} Dinf:{%v} Smhd:{%v} Vmhd:{%v}", b.Header, b.Stbl, b.Dinf, b.Smhd, b.Vmhd)
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
