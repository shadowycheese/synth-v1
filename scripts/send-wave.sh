#!/usr/bin/env bash

# Exit immediately if a command fails
set -e

# --- Configuration ---
# System Exclusive (SysEx) headers. 
# 0xF0 is the mandatory start byte. 0xF7 is the mandatory end byte.
# Adjust the manufacturer ID bytes (e.g., 7E 7F) to match your hardware specs.
SYS_START_1=0xF0
SYS_START_2=0x7E
SYS_START_3=0x7F
SYS_END=0xF7

# --- Validation ---
if [ "$#" -ne 2 ]; then
    echo "Error: Missing arguments." >&2
    echo "Usage: $0 <wave_id (0-5)> <wav_file>" >&2
    exit 1
fi

WAVE_ID="$1"
WAV_FILE="$2"

# Validate Wave ID range
if [[ ! "$WAVE_ID" =~ ^[0-5]$ ]]; then
    echo "Error: Wave ID must be an integer between 0 and 5." >&2
    exit 1
fi

# Verify file existence
if [ ! -f "$WAV_FILE" ]; then
    echo "Error: File '$WAV_FILE' not found." >&2
    exit 1
fi

# Verify strict file size (must be exactly 556 bytes)
FILE_SIZE=$(wc -c < "$WAV_FILE")
if [ "$FILE_SIZE" -ne 556 ]; then
    echo "Error: File size is $FILE_SIZE bytes. Expected exactly 556 bytes." >&2
    exit 1
fi

# --- Processing ---
# Convert the Wave ID to a 2-digit uppercase hex string
HEX_WAVE_ID=$(printf "%02X" "$WAVE_ID")

# Extract the 512 data bytes starting at byte 44 (skip 44 bytes of headers)
# xxd formats it into a continuous, uppercase hex string
HEX_DATA=$(tail -c +45 "$WAV_FILE" | xxd -p | tr -d '\n' | tr '[:lower:]' '[:upper:]')

# Verify the extracted data payload length (512 bytes = 1024 hex characters)
if [ "${#HEX_DATA}" -ne 1024 ]; then
    echo "Error: Failed to extract exactly 512 bytes of data." >&2
    exit 1
fi

MIDI_PORT=$(amidi -l | awk '/hw:/ {print $2; exit}')

if [ -z "$MIDI_PORT" ]; then
    echo "Error: No active USB MIDI devices found via 'amidi -l'." >&2
    rm -f "$TMP_BIN"
    exit 1
fi

function upload_chunk() {
    echo "Sending Wave ID $WAVE_ID ($WAV_FILE) chunk $1 via MIDI port: $MIDI_PORT"
    TMP_BIN=$(mktemp)

    printf "\\x$(printf %02X $SYS_START_1)\\x$(printf %02X $SYS_START_2)\\x$(printf %02X $SYS_START_3)" > "$TMP_BIN"
    printf "\\x$(printf %02X "$1")" >> "$TMP_BIN"
    printf "\\x$(printf %02X "$WAVE_ID")" >> "$TMP_BIN"
    printf "%s" $2 >> "$TMP_BIN"
    printf "\\x$(printf %02X $SYS_END)" >> "$TMP_BIN"

    if amidi -p $MIDI_PORT -s $TMP_BIN; then
        echo "Transmission successful!"
        rm -f "$TMP_BIN"
    else
        echo "Error: amidi transmission failed." >&2
        rm -f "$TMP_BIN"
        exit 1
    fi
}


upload_chunk 0 "${HEX_DATA:0:256}"
upload_chunk 1 "${HEX_DATA:256:256}"
upload_chunk 2 "${HEX_DATA:512:256}"
upload_chunk 3 "${HEX_DATA:768:256}"

