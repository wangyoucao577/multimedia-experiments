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

func (b *Boxes) CreateSubBox(h box.Header) (box.Box, error) {
	creator, ok := b.boxesCreator[h.Type.String()]
	if !ok {
		glog.Warningf("unknown box type %s, size %d payload %d", h.Type.String(), h.Size, h.PayloadSize())
		return nil, box.ErrUnknownBoxType
	}

	createdBox := creator(h)
	if createdBox == nil {
		glog.Fatalf("create box type %s failed", h.Type.String())
	}

	if h.Type.String() == box.TypeFtyp {
		b.Ftyp = createdBox.(*ftyp.Box)
	} else if h.Type.String() == box.TypeFree || h.Type.String() == box.TypeSkip {
		b.Free = append(b.Free, *createdBox.(*free.Box))
		createdBox = &b.Free[len(b.Free)-1] // reference to the last empty free box
	} else if h.Type.String() == box.TypeMdat {
		b.Mdat = createdBox.(*mdat.Box)
	} else if h.Type.String() == box.TypeMoov {
		b.Moov = createdBox.(*moov.Box)
	}

	return createdBox, nil
}
