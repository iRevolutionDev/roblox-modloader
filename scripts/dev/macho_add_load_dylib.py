#!/usr/bin/env python3

import struct
import sys

MH_MAGIC_64 = 0xFEEDFACF
LC_LOAD_DYLIB = 0x0C
LC_LOAD_WEAK_DYLIB = 0x80000018
LC_REEXPORT_DYLIB = 0x8000001F
LC_SEGMENT_64 = 0x19

HEADER_SIZE = 32
SEGMENT_COMMAND_64_SIZE = 72
SECTION_64_SIZE = 80
DYLIB_COMMAND_SIZE = 24

class MachoError(Exception):
    pass

def _u32(data, offset):
    return struct.unpack_from("<I", data, offset)[0]

def _require_thin64(data):
    if len(data) < HEADER_SIZE or _u32(data, 0) != MH_MAGIC_64:
        raise MachoError(
            "not a thin 64-bit Mach-O image (a universal/fat binary must be thinned first)"
        )

def loaded_dylibs(data):
    _require_thin64(data)

    ncmds = _u32(data, 16)
    cursor = HEADER_SIZE
    dylibs = []

    for _ in range(ncmds):
        if cursor + 8 > len(data):
            raise MachoError("the load commands are malformed")

        cmd = _u32(data, cursor)
        cmdsize = _u32(data, cursor + 4)
        if cmdsize < 8 or cursor + cmdsize > len(data):
            raise MachoError("the load commands are malformed")

        if cmd in (LC_LOAD_DYLIB, LC_LOAD_WEAK_DYLIB, LC_REEXPORT_DYLIB):
            name_offset = _u32(data, cursor + 8)
            if name_offset < DYLIB_COMMAND_SIZE or name_offset > cmdsize:
                raise MachoError("the load commands are malformed")
            raw = data[cursor + name_offset : cursor + cmdsize]
            end = raw.find(b"\x00")
            if end == -1:
                end = len(raw)
            dylibs.append(raw[:end].decode("utf-8", "replace"))

        cursor += cmdsize

    return dylibs

def _header_pad_ceiling(data, ncmds):
    cursor = HEADER_SIZE
    ceiling = None

    for _ in range(ncmds):
        if cursor + 8 > len(data):
            raise MachoError("the load commands are malformed")

        cmd = _u32(data, cursor)
        cmdsize = _u32(data, cursor + 4)
        if cmdsize < 8 or cursor + cmdsize > len(data):
            raise MachoError("the load commands are malformed")

        if cmd == LC_SEGMENT_64:
            nsects = _u32(data, cursor + 64)
            section = cursor + SEGMENT_COMMAND_64_SIZE
            for _ in range(nsects):
                if section + SECTION_64_SIZE > cursor + cmdsize:
                    raise MachoError("the load commands are malformed")
                offset = _u32(data, section + 48)
                if offset != 0:
                    ceiling = offset if ceiling is None else min(ceiling, offset)
                section += SECTION_64_SIZE

        cursor += cmdsize

    return len(data) if ceiling is None else ceiling

def _build_load_dylib_command(dylib_path):
    path = dylib_path.encode("utf-8")
    cmdsize = (DYLIB_COMMAND_SIZE + len(path) + 1 + 7) & ~7

    command = bytearray()
    command += struct.pack("<I", LC_LOAD_DYLIB)
    command += struct.pack("<I", cmdsize)
    command += struct.pack("<I", DYLIB_COMMAND_SIZE)
    command += struct.pack("<I", 2)
    command += struct.pack("<I", 0x00010000)
    command += struct.pack("<I", 0x00010000)
    command += path
    command += b"\x00" * (cmdsize - len(command))
    return bytes(command)

def insert_load_dylib(data, dylib_path):
    _require_thin64(data)

    ncmds = _u32(data, 16)
    sizeofcmds = _u32(data, 20)
    commands_end = HEADER_SIZE + sizeofcmds
    if commands_end > len(data):
        raise MachoError("the load commands are malformed")

    if dylib_path in loaded_dylibs(data):
        raise _AlreadyPresent(dylib_path)

    ceiling = _header_pad_ceiling(data, ncmds)
    command = _build_load_dylib_command(dylib_path)
    available = max(0, ceiling - commands_end)
    if len(command) > available:
        raise MachoError(
            "no room to insert a load command: need %d bytes but only %d are free in the header pad"
            % (len(command), available)
        )

    patched = bytearray(data)
    patched[commands_end : commands_end + len(command)] = command
    struct.pack_into("<I", patched, 16, ncmds + 1)
    struct.pack_into("<I", patched, 20, sizeofcmds + len(command))
    return bytes(patched)

class _AlreadyPresent(MachoError):
    def __init__(self, path):
        super().__init__("%s is already loaded by this image" % path)
        self.path = path

def _read(path):
    with open(path, "rb") as handle:
        return handle.read()

def main(argv):
    if len(argv) < 2:
        sys.stderr.write(__doc__)
        return 2

    mode = argv[1]

    try:
        if mode == "list":
            for path in loaded_dylibs(_read(argv[2])):
                print(path)
            return 0

        if mode == "has":
            return 0 if argv[3] in loaded_dylibs(_read(argv[2])) else 1

        if mode == "insert":
            binary, dylib_path = argv[2], argv[3]
            patched = insert_load_dylib(_read(binary), dylib_path)
            with open(binary, "wb") as handle:
                handle.write(patched)
            print("inserted LC_LOAD_DYLIB %s" % dylib_path)
            return 0

    except _AlreadyPresent as already:
        print("already loaded: %s" % already.path)
        return 3
    except (MachoError, IndexError, OSError) as error:
        sys.stderr.write("error: %s\n" % error)
        return 2

    sys.stderr.write("unknown mode: %s\n" % mode)
    return 2

if __name__ == "__main__":
    sys.exit(main(sys.argv))
