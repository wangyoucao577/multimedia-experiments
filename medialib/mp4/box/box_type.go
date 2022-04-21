package box

import (
	"bytes"
	"encoding/csv"
	"encoding/json"

	"github.com/ghodss/yaml"
)

// BasicInfo contains basic information of box, such as name, etc.
type BasicInfo struct {
	Name string `json:"name"`
}

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
	TypeStsd = "stsd"
	TypeStts = "stts"
	TypeStss = "stss"
	TypeStsc = "stsc"
	TypeStsz = "stsz"
	TypeStco = "stco"
	TypeCtts = "ctts"
	TypeDref = "dref"
	TypeUrl  = "url "
	TypeUrn  = "urn"
	TypeMoof = "moof"
	TypeMfhd = "mfhd"

	// sample entry
	TypeVide = "vide"
	TypeAvc1 = "avc1"
	TypeAvcC = "avcC"
	TypeBtrt = "btrt"
)

var boxTypes = map[string]BasicInfo{
	TypeUUID: {Name: "UUID"},

	TypeFtyp: {Name: "File Type Box"},
	TypeFree: {Name: "Free Space Box"},
	TypeSkip: {Name: "Free Space Box"},
	TypeMdat: {Name: "Media Data Box"},
	TypeMoov: {Name: "Movie Box"},
	TypeMvhd: {Name: "Movie Header Box"},
	TypeUdta: {Name: "User Data Box"},
	TypeCprt: {Name: "Copyright Box"},
	TypeMeta: {Name: "Meta Box"},
	TypeHdlr: {Name: "Handler Reference Box"},
	TypeIlst: {Name: "unknown"},
	TypeTrak: {Name: "Track Reference Box"},
	TypeTkhd: {Name: "Track Header Box"},
	TypeMdia: {Name: "Media Box"},
	TypeMdhd: {Name: "Media Header Box"},
	TypeMinf: {Name: "Media Information Box"},
	TypeStbl: {Name: "Sample Table Box"},
	TypeDinf: {Name: "Data Information Box"},
	TypeSmhd: {Name: "Sound Media Header"},
	TypeVmhd: {Name: "Video Media Header"},
	TypeStsd: {Name: "Sample Description Box"},
	TypeStts: {Name: "Decoding Time to Sample Box"},
	TypeStss: {Name: "Sync Sample Box"},
	TypeStsc: {Name: "Sample To Chunk Box"},
	TypeStsz: {Name: "Sample Size Box"},
	TypeStco: {Name: "Chunk Offset Box"},
	TypeCtts: {Name: "Composition Time to Sample Box"},
	TypeDref: {Name: "Data Reference Box"},
	TypeUrl:  {Name: "Data Entry Url Box"},
	TypeUrn:  {Name: "Data Entry Urn Box"},
	TypeMoof: {Name: "Movie Fragment Box"},
	TypeMfhd: {Name: "Movie Fragment Header Box"},

	TypeVide: {Name: "Visual Sample Entry"},
	TypeAvc1: {Name: "AVC Sample Entry"},
	TypeAvcC: {Name: "AVC Configuration Box"},
	TypeBtrt: {Name: "MPEG4 Bit Rate Box"},
}

// BoxTypes returns box types map.
func BoxTypes() map[string]BasicInfo {
	return boxTypes
}

// BoxTypesJSON marshalls supported boxes and relerrant information to JSON.
func BoxTypesJSON() ([]byte, error) {
	return json.Marshal(boxTypes)
}

// BoxTypesJSONIndent marshalls boxes to JSON representation with customized indent.
func BoxTypesJSONIndent(prefix, indent string) ([]byte, error) {
	return json.MarshalIndent(boxTypes, prefix, indent)
}

// BoxTypesYAML formats boxes to YAML representation.
func BoxTypesYAML() ([]byte, error) {
	j, err := json.Marshal(boxTypes)
	if err != nil {
		return j, err
	}
	return yaml.JSONToYAML(j)
}

// Print prints all supported boxes.
func BoxesTypesCSV() ([]byte, error) {
	records := [][]string{
		{"Type", "Name"}, // csv header
	}

	for k, v := range boxTypes {
		records = append(records, []string{k, v.Name})
	}

	buf := bytes.NewBuffer(nil)
	w := csv.NewWriter(buf)
	w.WriteAll(records)

	return buf.Bytes(), w.Error()
}

// IsValidBoxType checks whether input box type is valid or not.
func IsValidBoxType(t string) bool {
	_, ok := boxTypes[t]
	return ok
}
