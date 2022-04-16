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
)

var boxTypes = map[string]struct{}{
	TypeUUID: {},

	TypeFtyp: {},
	TypeFree: {},
	TypeSkip: {},
	TypeMdat: {},
	TypeMoov: {},
	TypeMvhd: {},
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
