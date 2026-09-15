#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
#
# Parse a Qualcomm OP-TEE DIAG log buffer extracted from a RAM/crash dump.
#
# The DIAG region lives in IMEM at:
#   DIAG_BASE = IMEM_BASE + IMEM_SIZE - DIAG_SIZE   (0x3000 bytes)
#
# Binary layout (matches diag_log.c):
#   struct diag_hdr   { uint32 version; uint32 magic; }
#   struct diag_conf  { uint32 buf_offset; uint32 buf_size; }
#   struct circ_wo_buf{ uint32 wrap; uint32 head; uint8 buf[]; }
#
# Usage:
#   # Auto-scan OCIMEM ramdump (no offset required):
#   python3 diag_log_parser.py --input OCIMEM.BIN --scan
#
#   # OCIMEM ramdump with known DIAG_BASE offset:
#   python3 diag_log_parser.py --input OCIMEM.BIN --offset 0x<diag_base_in_file>
#
#   # From a pre-extracted DIAG region (already sliced out of OCIMEM):
#   python3 diag_log_parser.py --input diag_region.bin
#
#   # Pipe into symbolize.py for address resolution:
#   python3 diag_log_parser.py --input OCIMEM.BIN --scan | \
#       python3 symbolize.py -d <path-to-tee.elf-dir>

import argparse
import os
import struct
import sys

DIAG_MAGIC_INIT   = 0x47414944  # "DIAG" little-endian — log valid
DIAG_MAGIC_FAILED = 0xDEADBEEF  # init failed
DIAG_MAGIC_DLOAD  = 0xD15AB1ED  # dload mode, log not written

MAGIC_NAMES = {
    DIAG_MAGIC_INIT:   'DIAG_MAGIC_INIT (valid)',
    DIAG_MAGIC_FAILED: 'DIAG_MAGIC_FAILED (init failed — buffer invalid)',
    DIAG_MAGIC_DLOAD:  'DIAG_MAGIC_DLOAD (dload mode — no log written)',
}

# struct offsets
OFF_VERSION    = 0       # uint32
OFF_MAGIC      = 4       # uint32
OFF_BUF_OFFSET = 8       # uint32
OFF_BUF_SIZE   = 12      # uint32
OFF_WRAP       = 16      # uint32
OFF_HEAD       = 20      # uint32
OFF_BUF_START  = 24      # uint8[] starts here (matches offsetof circ_wo_buf.buf)


SCAN_STEP = 0x1000  # DIAG_BASE is always page-aligned


def parse_args():
    p = argparse.ArgumentParser(
        description='Parse a Qualcomm OP-TEE DIAG log buffer from a crash dump',
        epilog='''
Examples:
  # Auto-scan OCIMEM ramdump for the DIAG magic (no need to know DIAG_BASE):
  python3 diag_log_parser.py --input OCIMEM.BIN --scan

  # OCIMEM ramdump with known DIAG_BASE offset:
  # (for cacao: IMEM_BASE=0x14680000, IMEM_SIZE=0x19000, DIAG_SIZE=0x3000
  #  => DIAG_BASE = 0x14680000 + 0x19000 - 0x3000 = 0x14696000)
  python3 diag_log_parser.py --input OCIMEM.BIN --offset 0x14696000

  # Bobcat uses DIAG_SIZE=0x6000 and a fixed DIAG_BASE=0x8608000:
  python3 diag_log_parser.py --input OCIMEM.BIN --offset 0x8608000 --size 0x6000

  # Pre-extracted DIAG region (already sliced out of OCIMEM):
  python3 diag_log_parser.py --input diag_region.bin

  # Pipe through symbolize.py:
  python3 diag_log_parser.py --input OCIMEM.BIN --scan | \\
      python3 symbolize.py -d out/cacao/
''',
        formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--input', '-i', required=True,
                   help='Binary file: pre-extracted DIAG region or full RAM dump')
    p.add_argument('--offset', '-o', default='0',
                   help='Byte offset of DIAG_BASE within the input file '
                        '(hex or decimal, default: 0)')
    p.add_argument('--size', '-s', default='0x3000',
                   help='DIAG region size in bytes (default: 0x3000)')
    p.add_argument('--scan', action='store_true',
                   help='Scan the input file for the DIAG magic instead of using --offset')
    p.add_argument('--verbose', '-v', action='store_true',
                   help='Print header fields before the log text')
    return p.parse_args()


def read_u32(data, offset):
    return struct.unpack_from('<I', data, offset)[0]


def scan_for_diag(path, size):
    """Scan path at SCAN_STEP increments for DIAG_MAGIC_INIT.

    Returns the file offset of the first match, or -1 if not found.
    Only considers a hit valid if the region starting there is large
    enough to hold the full header.
    """
    file_size = os.path.getsize(path)
    magic_bytes = struct.pack('<I', DIAG_MAGIC_INIT)

    print(f'Scanning {path} ({file_size:#x} bytes) for DIAG magic '
          f'0x{DIAG_MAGIC_INIT:08x} ...', file=sys.stderr)

    with open(path, 'rb') as f:
        offset = 0
        while offset + OFF_BUF_START <= file_size:
            f.seek(offset + OFF_MAGIC)
            word = f.read(4)
            if len(word) < 4:
                break
            if word == magic_bytes:
                # Verify enough room for a full DIAG region
                if offset + size <= file_size:
                    print(f'Found DIAG magic at file offset 0x{offset:x}',
                          file=sys.stderr)
                    return offset
            offset += SCAN_STEP

    print('DIAG magic not found in dump', file=sys.stderr)
    return -1


def main():
    args = parse_args()
    size = int(args.size, 0)

    if args.scan:
        offset = scan_for_diag(args.input, size)
        if offset < 0:
            sys.exit(1)
    else:
        offset = int(args.offset, 0)

    with open(args.input, 'rb') as f:
        f.seek(offset)
        data = f.read(size)

    if len(data) < OFF_BUF_START:
        print(f'ERROR: read only {len(data)} bytes — too small to contain header',
              file=sys.stderr)
        sys.exit(1)

    version    = read_u32(data, OFF_VERSION)
    magic      = read_u32(data, OFF_MAGIC)
    buf_offset = read_u32(data, OFF_BUF_OFFSET)
    buf_size   = read_u32(data, OFF_BUF_SIZE)
    wrap       = read_u32(data, OFF_WRAP)
    head       = read_u32(data, OFF_HEAD)

    if args.verbose:
        print(f'--- DIAG header ---', file=sys.stderr)
        print(f'  version    : {version}', file=sys.stderr)
        print(f'  magic      : 0x{magic:08x}  {MAGIC_NAMES.get(magic, "UNKNOWN")}',
              file=sys.stderr)
        print(f'  buf_offset : 0x{buf_offset:x}', file=sys.stderr)
        print(f'  buf_size   : 0x{buf_size:x} ({buf_size} bytes)', file=sys.stderr)
        print(f'  wrap       : {wrap}', file=sys.stderr)
        print(f'  head       : 0x{head:x} ({head})', file=sys.stderr)
        print(f'-------------------', file=sys.stderr)

    if magic == DIAG_MAGIC_FAILED:
        print(f'ERROR: DIAG magic = DIAG_MAGIC_FAILED — OP-TEE diag_log_init failed',
              file=sys.stderr)
        sys.exit(1)

    if magic == DIAG_MAGIC_DLOAD:
        print(f'ERROR: DIAG magic = DIAG_MAGIC_DLOAD — device was in dload mode, '
              f'no log was written', file=sys.stderr)
        sys.exit(1)

    if magic != DIAG_MAGIC_INIT:
        print(f'ERROR: unrecognised magic 0x{magic:08x} — '
              f'DIAG region may not be initialised or dump is corrupt',
              file=sys.stderr)
        sys.exit(1)

    # Sanity checks
    if buf_offset < OFF_BUF_START:
        print(f'ERROR: buf_offset(0x{buf_offset:x}) < header size (0x{OFF_BUF_START:x}) '
              f'— dump corrupt', file=sys.stderr)
        sys.exit(1)

    if buf_offset + buf_size > len(data):
        print(f'ERROR: buf_offset(0x{buf_offset:x}) + buf_size(0x{buf_size:x}) '
              f'exceeds region size(0x{len(data):x})', file=sys.stderr)
        sys.exit(1)

    if head >= buf_size:
        print(f'ERROR: head(0x{head:x}) > buf_size(0x{buf_size:x}) — dump corrupt',
              file=sys.stderr)
        sys.exit(1)

    buf = data[buf_offset: buf_offset + buf_size]

    # Reconstruct log order from circular buffer:
    #   wrap == 0: buffer has not wrapped, valid data is buf[0..head]
    #   wrap  > 0: buffer has wrapped, oldest data starts at head
    if wrap == 0:
        log_bytes = buf[:head]
    else:
        # oldest data: from head to end, then 0 to head
        log_bytes = buf[head:] + buf[:head]

    log_text = log_bytes.decode('utf-8', errors='replace')

    if args.verbose:
        total = head if wrap == 0 else buf_size
        print(f'--- log ({total} bytes, wrap={wrap}) ---', file=sys.stderr)

    sys.stdout.write(log_text)
    if log_text and not log_text.endswith('\n'):
        sys.stdout.write('\n')


if __name__ == '__main__':
    main()
