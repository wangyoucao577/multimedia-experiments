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
	flag.StringVar(&flags.content, "content", "mp4", "Contents to parse and output, available values: \nbox-types: supported boxes(no parse) \nnalu-types: NALU types(no parse)  \nes: AVC/HEVC elementary stream parsing data \nraw-es: AVC/HEVC elementary stream(mp4 video elementary stream only, no sps/pps) \nboxes: MP4 boxes")
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
	flagContentRawES        // AVC/HEVC Elementary Stream (mp4 video elementary stream only, no sps/pps)

	// no parse needed
	flagContentBoxTypes
	flagContentNALUTypes
)

func getContentFlag() int {
	switch strings.ToLower(flags.content) {
	case "es":
		return flagContentES
	case "raw-es":
		return flagContentRawES

	case "box-types":
		return flagContentBoxTypes
	case "nalu-types":
		return flagContentNALUTypes
	}
	return flagContentBoxes
}
