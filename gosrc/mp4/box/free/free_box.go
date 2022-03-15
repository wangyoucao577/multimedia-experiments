// Package free represents free-space boxes which may has type `free` or `skip`.
package free

import (
	"fmt"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/gosrc/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/gosrc/util"
)

// Box represents a ftyp box.
type Box struct {
	box.Header

	Data []byte
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Header:{%v} Data:%s", b.Header, string(b.Data))
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {
	if b.PayloadSize() > 0 {
		b.Data = make([]byte, b.PayloadSize())
		if err := util.ReadOrError(r, b.Data[:]); err != nil {
			return err
		}
	}
	return nil
}
