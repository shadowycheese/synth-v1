#!/usr/bin/env bash

# IMPORTANT: This script requires ffmpeg + amidi to be installed

set -e

DEPENDENCIES=("ffmpeg" "amidi")
MISSING_DEPS=0

# Loop through and check each application
for app in "${DEPENDENCIES[@]}"; do
    if ! command -v "$app" &> /dev/null; then
        if [ "$MISSING_DEPS" -eq 0 ]; then
            echo "Error:" >&2
        fi
        MISSING_DEPS=$((MISSING_DEPS + 1))
        echo "  $app is not installed or not in your PATH" >&2
    fi
done

# If anything was missing, exit early before running the rest of the script
if [ "$MISSING_DEPS" -gt 0 ]; then
    exit 1
fi

MIDI_PORT=$(amidi -l | grep -i Teensy | awk '/hw:/ {print $2; exit}')

if [ -z "$MIDI_PORT" ]; then    
    echo "Error:" >&2
    echo "  No active USB MIDI devices found via 'amidi -l'." >&2
    exit 1
fi

SYS_START_1=0xF0
SYS_START_2=0x7E
SYS_START_3=0x7F
SYS_END=0xF7
WAV_SAMPLES=256

WAVE_ID=
WAV_FILE=
HEX_FILE=
BIN_FILE=
WAV_TYPE=
HEX_DATA=

function usage() {
    echo "Usage:"
    echo '  send-wave <arguments>'
    echo 
    echo "  -t <waveform_id> The waveform storage location (0-5)"
    echo "  -s <hex_string>  The waveform in HEX. Must be exactly 1024 characters"
    echo "  -w <wav_file>    A .wav file containing a single audio frame with exactly $WAV_SAMPLES 16 bit signed values"
    echo "  -h <hex_file>    A file containing 1024 hex characters (whitepsace is allowed)"
    echo "  -b <bin_file>    A binary file containing exactly $WAV_SAMPLES 16 bit signed values"
    exit 0
}
function upload_chunk() {
    tmp_bin=$(mktemp)

    printf "\\x$(printf %02X $SYS_START_1)\\x$(printf %02X $SYS_START_2)\\x$(printf %02X $SYS_START_3)" > "$tmp_bin"
    printf "\\x$(printf %02X "$1")" >> "$tmp_bin"
    printf "\\x$(printf %02X "$WAVE_ID")" >> "$tmp_bin"
    printf "%s" $2 >> "$tmp_bin"
    printf "\\x$(printf %02X $SYS_END)" >> "$tmp_bin"

    if amidi -p $MIDI_PORT -s $tmp_bin; then
        rm -f "$tmp_bin"
    else
        printf "FAILED\n"
        echo "Error:" >&2
        echo "  amidi transmission failed." >&2
        rm -f "$tmp_bin"
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

function load_wav_file() {
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

    if [[ "$channels" -gt 2 ]]; then
        echo "Error:" >&2
        echo "  Only mono/stereo .wav files are supported (Channels: $channels)" >&2
        exit 1
    fi

    tmp_wav=$(mktemp)

    if [[ "$samples" -ne $WAV_SAMPLES ]]; then
        target_rate=$(awk "BEGIN {print int(($WAV_SAMPLES / $samples) * $samples)}")

        ffmpeg_af="asetrate=44100*$WAV_SAMPLES/$samples,aresample=44100,atrim=end_sample=$WAV_SAMPLES"

        if [[ "$channels" -eq 2 ]]; then
            echo "Resampling stereo $WAV_FILE: ${samples}Hz => ${WAV_SAMPLES}Hz"
            ffmpeg_af="pan=mono|c0=0.5*c0+0.5*c1,${ffmpeg_af}"
        else
            echo "Resampling $WAV_FILE: ${samples}Hz => ${WAV_SAMPLES}Hz"
        fi

        ffmpeg -y -v error -i "$WAV_FILE" -af "$ffmpeg_af" -frames:a 1 -f wav $tmp_wav
    else 
        cp $WAV_FILE $tmp_wav
    fi

    HEX_DATA=$(ffmpeg -v error -i "$tmp_wav" -f s16be - | od -An -v -tx1 | tr -d ' \n')

    rm -f $tmp_wav

    error_msg="(read from .wav file $WAV_FILE)\n"

    validate_hex_string $error_msg
}

function load_hex_file() {
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

function load_bin_file() {
    if [ ! -f "$BIN_FILE" ]; then
        echo "Error:" >&2
        echo "  File '$BIN_FILE' not found." >&2
        exit 1
    fi      

    HEX_DATA=$(cat "$BIN_FILE" | xxd -p | tr -d '\n' | tr '[:lower:]' '[:upper:]')

    error_msg="(read from binary file $BIN_FILE)\n"

    validate_hex_string $error_msg
}

function load_c_file() {
    HEX_DATA=$(sed -n '/{/,/}/p' "$C_FILE" |
        grep -oE -- '-?[0-9]+' |
        while read -r n; do
            printf "%04X" $(( n & 0xFFFF ))
        done)

    error_msg="(read 'C' variable in file $C_FILE)\n"

    validate_hex_string $error_msg
}

if [ "$1" == "" ]; then
    usage
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
    -c)
      C_FILE=$2
      WAV_TYPE=C_FILE
      ;;
    --help)
      usage
      exit 0
  esac
  shift
done

validate_wave_id

if [ "$WAV_TYPE" == 'WAV_FILE' ]; then
    load_wav_file 
else if [ "$WAV_TYPE" == 'HEX_FILE' ]; then
    load_hex_file 
else if [ "$WAV_TYPE" == 'BIN_FILE' ]; then
    load_bin_file 
else if [ "$WAV_TYPE" == 'C_FILE' ]; then
    load_c_file
else if [ "$WAV_TYPE" == 'HEX_DATA' ]; then
    validate_hex_string "(command line)"
else 
    echo "Error:" >&2
    echo "  No input type specified" >&2
    exit 1
fi fi fi fi fi

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
echo "Waveform data uploaded: Location=$WAVE_ID [MIDI=$MIDI_PORT]"