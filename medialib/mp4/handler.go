//Package mp4 defines mp4 handlers and structures.
package mp4

import (
	"fmt"
	"io"
	"os"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/free"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/ftyp"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/mdat"
)

// Handler represents handler for `mp4` structure.
type Handler struct {
	Boxes

	f        *os.File
	filePath string

	boxesCreator map[string]box.NewFunc
}

// New creates mp4 Handler.
func New(filePath string) *Handler {
	return &Handler{
		filePath: filePath,

		boxesCreator: map[string]box.NewFunc{
			box.TypeFtyp: ftyp.New,
			box.TypeFree: free.New,
			box.TypeSkip: free.New,
			box.TypeMdat: mdat.New,
		},
	}
}

// return error indicates whether box type unregistered.
func (h *Handler) createBox(bh box.Header) (box.Box, error) {

	creator, ok := h.boxesCreator[bh.Type.String()]
	if !ok {
		return nil, fmt.Errorf("unregistered box type %s, size %d payload %d", bh.Type.String(), bh.Size, bh.PayloadSize())
	}

	b := creator(bh)
	if b == nil {
		glog.Fatalf("create box type %s failed", bh.Type.String())
	}

	if bh.Type.String() == box.TypeFtyp {
		h.Ftyp = b.(*ftyp.Box)
	} else if bh.Type.String() == box.TypeFree || bh.Type.String() == box.TypeSkip {
		h.Free = append(h.Free, *b.(*free.Box))
		b = &h.Free[len(h.Free)-1] // reference to the last empty free box
	} else if bh.Type.String() == box.TypeMdat {
		h.Mdat = b.(*mdat.Box)
	}

	return b, nil
}

// Parse parses mp4 file.
func (h *Handler) Parse() error {

	if err := h.open(); err != nil {
		glog.Warningf("open %s failed, err %v", h.filePath, err)
		return err
	}
	defer h.close()

	for {
		boxHeader := box.Header{}
		if err := boxHeader.Parse(h.f); err != nil {
			if err == io.EOF {
				break
			}
			glog.Warningf("parse box header failed, err %v", err)
			return err
		}

		b, err := h.createBox(boxHeader)
		if err != nil {
			//TODO: other types
			glog.Warningf("ignore %v", err)
			h.f.Seek(int64(boxHeader.PayloadSize()), 1)
			continue
		}

		if err := b.ParsePayload(h.f); err != nil {
			glog.Warningf("parse box type %s payload failed, err %v", string(boxHeader.Type[:]), err)
			return err
		}

		if boxHeader.Size == 0 {
			break // last box has been parsed
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
