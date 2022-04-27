package main

import (
	"flag"
	"strings"
)

var flags struct {
	inputFilePath string
	content       string // content to output
	format        string
}

func init() {
	flag.StringVar(&flags.inputFilePath, "i", "", "Input mp4 file path.")
	flag.StringVar(&flags.content, "content", "boxes", `Contents to parse and output, available values: 
  box_types: supported boxes(no parse) 
  nalu_types: NALU types(no parse)  
  es: AVC/HEVC elementary stream parsing data 
  raw_es: AVC/HEVC elementary stream(mp4 video elementary stream only, no sps/pps) 
  raw_annexb_es: AVC/HEVC Elementary Stream (AnnexB byte format, video elementary stream and parameter set elementary stream) 
  boxes: MP4 boxes`)
	flag.StringVar(&flags.format, "format", "json", "Output format, available values:json,json_newlines,yaml,csv. \nNote that 'csv' only available for 'no parse' content")
}

const (
	flagFormatJSON = iota
	flagFormatNewLines
	flagFormatYAML
	flagFormatCSV
)

func getFormatFlag() int {
	switch strings.ToLower(flags.format) {
	case "yaml":
		return flagFormatYAML
	case "json_newlines":
		return flagFormatNewLines
	case "csv":
		return flagFormatCSV
	}
	return flagFormatJSON
}

const (
	flagContentBoxes       = iota // mp4 boxes
	flagContentES                 // AVC/HEVC Elementary Stream parsing data
	flagContentRawES              // AVC/HEVC Elementary Stream (mp4 video elementary stream only, no sps/pps)
	flagContentRawAnnexBES        // AVC/HEVC Elementary Stream (AnnexB byte format, video elementary stream and parameter set elementary stream)

	// no parse needed
	flagContentBoxTypes
	flagContentNALUTypes
)

func getContentFlag() int {
	switch strings.ToLower(flags.content) {
	case "es":
		return flagContentES
	case "raw_es":
		return flagContentRawES
	case "raw_annexb_es":
		return flagContentRawAnnexBES

	case "box_types":
		return flagContentBoxTypes
	case "nalu_types":
		return flagContentNALUTypes
	}
	return flagContentBoxes
}
