// Package moov defines moov box structure which contains meta data.
package moov

import (
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/mvhd"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/trak"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/udta"
)

// Box represents a mdat box.
type Box struct {
	box.Header

	Mvhd *mvhd.Box
	Udta *udta.Box
	Trak []trak.Box

	boxesCreator map[string]box.NewFunc
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		Header: h,

		boxesCreator: map[string]box.NewFunc{
			box.TypeMvhd: mvhd.New,
			box.TypeUdta: udta.New,
			box.TypeTrak: trak.New,
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
	case box.TypeMvhd:
		b.Mvhd = createdBox.(*mvhd.Box)
	case box.TypeUdta:
		b.Udta = createdBox.(*udta.Box)
	case box.TypeTrak:
		b.Trak = append(b.Trak, *createdBox.(*trak.Box))
		createdBox = &b.Trak[len(b.Trak)-1] // reference to the last empty free box
	}

	return createdBox, nil
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Header:{%v} mvhd:{%v} udta:{%v} Trak:{%v}", b.Header, b.Mvhd, b.Udta, b.Trak)
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {

	payloadSize := b.PayloadSize()

	if payloadSize == 0 {
		return fmt.Errorf("box %s is empty", b.Type)
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

		if parsedBytes == payloadSize {
			break
		}
	}

	return nil
}
