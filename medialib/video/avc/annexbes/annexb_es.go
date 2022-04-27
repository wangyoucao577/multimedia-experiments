// Package annexbes represents Annex B defined AVC Elementary byte stream,
// which was defined in ISO/IEC-14496-19 Annex B.
package annexbes

import (
	"fmt"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/medialib/video/avc/nalu"
)

// ElementaryStream represents AVC Elementary Stream.
type ElementaryStream struct {
	NALU []nalu.NALUnit `json:"nalu"`
}

// Dump dumps raw data into io.Writer.
func (e *ElementaryStream) Dump(w io.Writer) (int, error) {
	if len(e.NALU) == 0 {
		return 0, fmt.Errorf("empty elementary stream")
	}

	var writedBytes int

	for i := range e.NALU {
		data := []byte{0x00, 0x00, 0x00, 0x01} // Annex B start code
		if n, err := w.Write(data); err != nil {
			return writedBytes, err
		} else if n != len(data) {
			return writedBytes, fmt.Errorf("write bytes unmatch, expect(%d) != actual(%d)", len(data), n)
		} else {
			writedBytes += n
		}

		rsbp := e.NALU[i].Raw()
		if n, err := w.Write(rsbp); err != nil {
			return writedBytes, err
		} else if n != len(rsbp) {
			return writedBytes, fmt.Errorf("write bytes unmatch, expect(%d) != actual(%d)", len(data), n)
		} else {
			writedBytes += n
		}
	}

	return writedBytes, nil
}
