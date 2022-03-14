//Package mp4 defines mp4 handlers and structures.
package mp4

import (
	"io"
	"os"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/gosrc/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/gosrc/mp4/box/free"
	"github.com/wangyoucao577/multimedia-experiments/gosrc/mp4/box/ftyp"
)

// Handler represents handler for `mp4` structure.
type Handler struct {
	Boxes

	f        *os.File
	filePath string
}

// New creates mp4 Handler.
func New(filePath string) *Handler {
	return &Handler{
		filePath: filePath,
	}
}

// Parse parses mp4 file.
func (h *Handler) Parse() error {

	if err := h.open(); err != nil {
		glog.Warningf("open %s failed, err %v", h.filePath, err)
		return err
	}
	defer h.close()

	for {
		basicBox := box.Box{}
		if err := basicBox.Parse(h.f); err != nil {
			if err == io.EOF {
				break
			}
			glog.Warningf("parse box failed, err %v", err)
			return err
		}

		typeStr := string(basicBox.Type[:])
		if typeStr == box.TypeFtyp {
			h.Ftyp = &ftyp.Box{
				Box: basicBox,
			}
			if err := h.Ftyp.ParsePayload(h.f); err != nil {
				glog.Warningf("parse ftyp box failed, err %v", err)
				return err
			}
		} else if typeStr == box.TypeFree || typeStr == box.TypeSkip {
			freeBox := free.Box{
				Box: basicBox,
			}
			if err := freeBox.ParsePayload(h.f); err != nil {
				glog.Warningf("parse free box failed, err %v", err)
				return err
			}
			h.Free = append(h.Free, freeBox)
		} else {
			//TODO: other types
			glog.Infof("ignore unknown type %s, size %d payload %d", basicBox.Type, basicBox.Size, basicBox.PayloadSize())
			h.f.Seek(int64(basicBox.PayloadSize()), 1)
		}

		if basicBox.Size == 0 {
			break
		}
	}

	return nil
}

// Open opens mp4 file.
func (h *Handler) open() error {

	var err error
	if h.f, err = os.Open(h.filePath); err != nil {
		return err
	}
	glog.V(1).Infof("open %s succeed.\n", h.filePath)

	return nil
}

// Close closes the mp4 file handler.
func (h *Handler) close() error {
	if h == nil || h.f == nil {
		return nil
	}

	return h.f.Close()
}
