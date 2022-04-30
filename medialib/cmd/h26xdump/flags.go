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
	flag.StringVar(&flags.inputFilePath, "i", "", `Input Elementary Stream file path, such as 'x.h264' or 'x.h265'.
Be aware that the Elementary Stream file is mandatory stored by AnnexB byte stream format.`)
	flag.StringVar(&flags.content, "content", "es", `Contents to parse and output, available values: 
  nalu_types: NALU types(no parse)  
  es: AVC/HEVC elementary stream parsing data`)
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
	flagContentBoxes = iota // mp4 boxes
	flagContentES           // AVC/HEVC Elementary Stream parsing data

	// no parse needed
	flagContentNALUTypes
)

func getContentFlag() int {
	switch strings.ToLower(flags.content) {
	case "es":
		return flagContentES

	case "nalu_types":
		return flagContentNALUTypes
	}
	return flagContentBoxes
}
