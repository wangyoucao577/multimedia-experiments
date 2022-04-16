// Package moov defines moov box structure which contains meta data.
package moov

import (
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/moov/mvhd"
)

// Box represents a mdat box.
type Box struct {
	box.Header

	Mvhd *mvhd.Box

	boxesCreator map[string]box.NewFunc
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		Header: h,

		boxesCreator: map[string]box.NewFunc{
			box.TypeMvhd: mvhd.New,
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

	if h.Type.String() == box.TypeMvhd {
		b.Mvhd = createdBox.(*mvhd.Box)
	}
	//TODO: other types

	return createdBox, nil
}

// GetHeader return header of the box.
func (b Box) GetHeader() box.Header {
	return b.Header
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Header:{%v} mvhd:{%v}", b.Header, b.Mvhd)
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
