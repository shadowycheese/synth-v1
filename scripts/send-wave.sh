#!/usr/bin/env bash

# IMPORTANT: This script requires ffmpeg + amidi to be installed

set -e

SYS_START_1=0xF0
SYS_START_2=0x7E
SYS_START_3=0x7F
SYS_END=0xF7

WAVE_ID=
WAV_FILE=
HEX_FILE=
BIN_FILE=
WAV_TYPE=
HEX_DATA=

function upload_chunk() {
    printf "Sending Wave ID $WAVE_ID ($WAV_FILE) chunk $1 via MIDI port: $MIDI_PORT..."
    TMP_BIN=$(mktemp)

    printf "\\x$(printf %02X $SYS_START_1)\\x$(printf %02X $SYS_START_2)\\x$(printf %02X $SYS_START_3)" > "$TMP_BIN"
    printf "\\x$(printf %02X "$1")" >> "$TMP_BIN"
    printf "\\x$(printf %02X "$WAVE_ID")" >> "$TMP_BIN"
    printf "%s" $2 >> "$TMP_BIN"
    printf "\\x$(printf %02X $SYS_END)" >> "$TMP_BIN"

    if amidi -p $MIDI_PORT -s $TMP_BIN; then
        printf "COMPLETE\n"
        rm -f "$TMP_BIN"
    else
        printf "FAILED\n"
        echo "Error:" >&2
        echo "  amidi transmission failed." >&2
        rm -f "$TMP_BIN"
        exit 1
    fi
}

function validate_wave_type() {
    if [[ "$WAVE_TYPE" == "" ]]; then
        echo "Error:" >&2
        echo "  Must specify a hex string or a wav type." >&2
        exit 1
    fi
}

function validate_wave_id() {
    if [[ ! "$WAVE_ID" =~ ^[0-5]$ ]]; then
        echo "Error:" >&2
        echo "  Wave ID must be an integer between 0 and 5." >&2
        exit 1
    fi

    HEX_WAVE_ID=$(printf "%02X" "$WAVE_ID")
}

function validate_hex_string() {
    hex_size=${#HEX_DATA}
    if [ "$hex_size" -ne 1024 ]; then
        echo "Error:" >&2
        echo "  Hex data must contain exactly 1024 hex digits [was $hex_size]" >&2
        printf "  $*\n" >&2
        exit 1
    fi      

    if [[ ! "$HEX_DATA" =~ ^[0-9a-fA-F]+$ ]]; then
        echo "Error:" >&2
        printf "  $*\n" >&2
        echo "  Hex data must match [0-9a-fA-F]" >&2
        exit 1
    fi
}

function validate_wav_file() {
    if [ ! -f "$WAV_FILE" ]; then
        echo "Error:" >&2
        echo "  File '$WAV_FILE' not found." >&2
        exit 1
    fi      

    metadata=$(ffprobe -v error -show_entries stream=codec_name,sample_fmt,channels,duration_ts -of default=noprint_wrappers=1:nokey=1 "$WAV_FILE" 2>/dev/null)

    readarray -t metadata_arr <<< "$metadata"

    codec="${metadata_arr[0]}"
    fmt="${metadata_arr[1]}"
    channels="${metadata_arr[2]}"
    samples="${metadata_arr[3]}"

    if [[ ! "$codec" =~ pcm_s16 ]]; then
        echo "Error:" >&2
        echo "  Audio codec is not 16-bit signed PCM (Was: $codec)" >&2
        exit 1
    fi

    if [[ "$channels" -ne 1 ]]; then
        echo "Error:" >&2
        echo "  File is not mono (Channels: $channels)" >&2
        exit 1
    fi

    if [[ "$samples" -ne 256 ]]; then
        echo "Error:" >&2
        echo "  File does not contain exactly 256 samples (Got: $samples)" >&2
        exit 1
    fi

    HEX_DATA=$(ffmpeg -v error -i "$WAV_FILE" -f s16be - | od -An -v -tx1 | tr -d ' \n')

    echo $HEX_DATA
    error_msg="(read from .wav file $WAV_FILE)\n"

    validate_hex_string $error_msg
}

function validate_hex_file() {
    if [ ! -f "$HEX_FILE" ]; then
        echo "Error:" >&2
        echo "  File '$HEX_FILE' not found." >&2
        exit 1
    fi      

    # File size must be > 1024 bytes as the min number of hex digits
    HEX_DATA=$(cat "$HEX_FILE" | tr -d '[:space:]')

    error_msg="(read from HEX file $HEX_FILE)\n"

    validate_hex_string $error_msg
}

function validate_bin_file() {
    if [ ! -f "$BIN_FILE" ]; then
        echo "Error:" >&2
        echo "  File '$BIN_FILE' not found." >&2
        exit 1
    fi      

    HEX_DATA=$(cat "$BIN_FILE" | xxd -p | tr -d '\n' | tr '[:lower:]' '[:upper:]')

    error_msg="(read from binary file $BIN_FILE)\n"

    validate_hex_string $error_msg
}

if [ "$1" == "" ]; then
    echo "Usage:"
    echo '  send-wave <arguments>'
    echo 
    echo "  -t <waveform_id> The waveform storage location (0-5)"
    echo "  -s <hex_string>  The waveform in HEX. Must be exactly 1024 characters"
    echo "  -w <wav_file>    A .wav file containing exactly 256 16 bit signed values"
    echo "  -h <hex_file>    A file containing 1024 hex characters (whitepsace is allowed)"
    echo "  -b <bin_file>    A binary file containing exactly 256 16 bit signed values"
    exit 0
fi

# Parse params
while [[ $# > 0 ]] ; do
  case "$1" in
    -t)
      WAVE_ID=$2
      shift
      ;;
    -w)
      WAV_FILE=$2
      WAV_TYPE=WAV_FILE
      shift
      ;;
    -s)
      HEX_DATA=$2
      WAV_TYPE=HEX_DATA
      shift
      ;;
    -h)
      HEX_FILE=$2
      WAV_TYPE=HEX_FILE
      shift
      ;;
    -b)
      BIN_FILE=$2
      WAV_TYPE=BIN_FILE
      shift
      ;;
  esac
  shift
done

validate_wave_id

if [ "$WAV_TYPE" == 'WAV_FILE' ]; then
    validate_wav_file 
else if [ "$WAV_TYPE" == 'HEX_FILE' ]; then
    validate_hex_file 
else if [ "$WAV_TYPE" == 'BIN_FILE' ]; then
    validate_bin_file 
else if [ "$WAV_TYPE" == 'HEX_DATA' ]; then
    validate_hex_string "(command line)"
else 
    echo "Error:" >&2
    echo "  No input type specified" >&2
    exit 1
fi fi fi fi

MIDI_PORT=$(amidi -l | grep -i Teensy | awk '/hw:/ {print $2; exit}')

if [ -z "$MIDI_PORT" ]; then    
    echo "Error:" >&2
    echo "  No active USB MIDI devices found via 'amidi -l'." >&2
    exit 1
fi

upload_chunk 0 "${HEX_DATA:0:256}"
upload_chunk 1 "${HEX_DATA:256:256}"
upload_chunk 2 "${HEX_DATA:512:256}"
upload_chunk 3 "${HEX_DATA:768:256}"

