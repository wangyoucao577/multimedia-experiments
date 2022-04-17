package box

// Box types
const (
	TypeUUID = "uuid"

	TypeFtyp = "ftyp"
	TypeFree = "free"
	TypeSkip = "skip"
	TypeMdat = "mdat"
	TypeMoov = "moov"
	TypeMvhd = "mvhd"
	TypeUdta = "udta"
	TypeCprt = "cprt"
	TypeMeta = "meta"
	TypeHdlr = "hdlr"
	TypeIlst = "ilst"
	TypeTrak = "trak"
	TypeTkhd = "tkhd"
	TypeMdia = "mdia"
	TypeMdhd = "mdhd"
	TypeMinf = "minf"
	TypeStbl = "stbl"
	TypeDinf = "dinf"
	TypeSmhd = "smhd"
	TypeVmhd = "vmhd"
)

var boxTypes = map[string]struct{}{
	TypeUUID: {},

	TypeFtyp: {},
	TypeFree: {},
	TypeSkip: {},
	TypeMdat: {},
	TypeMoov: {},
	TypeMvhd: {},
	TypeUdta: {},
	TypeCprt: {},
	TypeMeta: {},
	TypeHdlr: {},
	TypeIlst: {},
	TypeTrak: {},
	TypeTkhd: {},
	TypeMdia: {},
	TypeMdhd: {},
	TypeMinf: {},
	TypeStbl: {},
	TypeDinf: {},
	TypeSmhd: {},
	TypeVmhd: {},
}

// BoxTypes returns box types map.
func BoxTypes() map[string]struct{} {
	return boxTypes
}

// IsValidBoxType checks whether input box type is valid or not.
func IsValidBoxType(t string) bool {
	_, ok := boxTypes[t]
	return ok
}
