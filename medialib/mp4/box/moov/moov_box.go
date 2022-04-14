// Package moov defines moov box structure which contains meta data.
package moov

import (
	"fmt"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
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

// GetHeader return header of the box.
func (b Box) GetHeader() box.Header {
	return b.Header
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Header:{%v}", b.Header)
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

// CreateSubBox tries to create sub level box.
func (b *Box) CreateSubBox(h box.Header) (box.Box, error) {
	//TODO:
	return nil, box.ErrUnknownBoxType
}
