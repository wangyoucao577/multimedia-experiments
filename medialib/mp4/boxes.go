package mp4

import (
	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/free"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/ftyp"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/mdat"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box/moov"
)

// Boxes represents mp4 boxes.
type Boxes struct {
	Ftyp *ftyp.Box
	Free []free.Box
	Mdat *mdat.Box
	Moov *moov.Box

	//TODO: other boxes

	// internal vars for parsing or other handling
	boxesCreator map[string]box.NewFunc
}

func newBoxes() Boxes {
	return Boxes{
		boxesCreator: map[string]box.NewFunc{
			box.TypeFtyp: ftyp.New,
			box.TypeFree: free.New,
			box.TypeSkip: free.New,
			box.TypeMdat: mdat.New,
			box.TypeMoov: moov.New,
		},
	}
}

// CreateSubBox creates directly included box, such as create `mvhd` in `moov`, or create `moov` on top level.
//   return ErrNotImplemented is the box doesn't have any sub box.
func (b *Boxes) CreateSubBox(h box.Header) (box.Box, error) {
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
	case box.TypeFtyp:
		b.Ftyp = createdBox.(*ftyp.Box)
	case box.TypeFree, box.TypeSkip:
		b.Free = append(b.Free, *createdBox.(*free.Box))
		createdBox = &b.Free[len(b.Free)-1] // reference to the last empty free box
	case box.TypeMdat:
		b.Mdat = createdBox.(*mdat.Box)
	case box.TypeMoov:
		b.Moov = createdBox.(*moov.Box)
	}

	return createdBox, nil
}
