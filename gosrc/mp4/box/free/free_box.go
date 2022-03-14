package free

import (
	"fmt"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/gosrc/mp4/box"
)

// Box represents a ftyp box.
type Box struct {
	box.Box
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Box:{%v}", b.Box)
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {
	// NOTE: it supposed to be 0 payload
	if b.PayloadSize() > 0 {
		return fmt.Errorf("invalid payload size %d", b.PayloadSize())
	}
	return nil
}
