"""Build DS Style for EZ-FLASH Omega Definitive Edition B."""
import argparse
import hashlib
import os
from pathlib import Path
import struct
import subprocess
import zlib

ROOT = Path(__file__).resolve().parents[1]
TAIL = 0x100000
FW = 0x195000
FW_SIZE = 0x716A0
FW_CRC = 0x22DD74FA
VENDOR_SHA256 = "be609b61939eaa19b97b81345f8e44e7c97b819fb76d230553d40f2d28688644"


def sha(data):
    return hashlib.sha256(data).hexdigest()


def word(data, offset):
    return struct.unpack_from("<I", data, offset)[0]


def patch_entry(payload, boot, wrapper):
    data = bytearray(payload)
    if (word(data,0) != 0xEA00002E or word(data,0xE0) != 0xE3A00301 or
            data[0x126:0x130] != bytes.fromhex("40210903c80100f02cf8")):
        raise RuntimeError("Unsupported devkitARM CRT startup")
    if boot.count(struct.pack("<I",0x12345678)) != 1 or boot.count(struct.pack("<I",0x23456789)) != 1:
        raise RuntimeError("Invalid boot trampoline")
    boot = boot.replace(struct.pack("<I",0x12345678),struct.pack("<I",0x080000E4))
    boot = boot.replace(struct.pack("<I",0x23456789),struct.pack("<I",0x08000189))
    data += b"\0" * (-len(data) % 4)
    offset = len(data)
    data += boot
    disp = offset + wrapper - 0x130
    if disp & 1 or not 0 <= disp < 0x400000:
        raise RuntimeError("CRT trampoline is out of range")
    struct.pack_into("<I",data,0xE0,0xEA000000 | ((offset - 0xE8) // 4))
    struct.pack_into("<HH",data,0x12C,0xF000 | (disp >> 12),0xF800 | ((disp >> 1) & 0x7FF))
    return data


def pack(payload, vendor):
    if len(payload) >= FW:
        raise RuntimeError("Kernel overlaps the fixed Blue firmware payload")
    if sha(vendor) != VENDOR_SHA256:
        raise RuntimeError("Wrong vendor image. Use image.bin from the pinned Blue source.")
    image = bytearray(TAIL + len(vendor))
    image[TAIL:] = vendor
    image[:len(payload)] = payload
    if zlib.crc32(image[FW:FW + FW_SIZE]) != FW_CRC or image[FW:] != vendor[FW - TAIL:]:
        raise RuntimeError("The vendor firmware region changed")
    return image


def symbols(nm, path):
    found = {}
    for line in subprocess.check_output([str(nm), "-n", str(path)], text=True).splitlines():
        parts = line.split()
        if len(parts) == 3:
            found[parts[2]] = int(parts[0], 16)
    return found


def build_text_viewer(run, gcc, objcopy, gbafix, arch, gba, output):
    source = ROOT / "tools/text-viewer/source"
    toolchain = source.parent / "toolchain"
    objects = []
    for name in ("main.c", "return_to_ds_style.s"):
        obj = output / f"text-{Path(name).stem}.o"
        run(gcc, *arch, "-Os", "-ffunction-sections", "-fdata-sections",
            "-I", gba / "include", "-c", source / name, "-o", obj)
        objects.append(obj)
    crt = output / "gba_crt0_dsstyle.o"
    run(gcc, *arch, "-c", toolchain / "gba_crt0_dsstyle.s", "-o", crt)
    elf, rom = output / "text-viewer.elf", output / "txt.bin"
    run(gcc, *arch, "-B" + str(output) + os.sep,
        "-specs=" + str(toolchain / "gba_dsstyle.specs"), "-T", toolchain / "gba_cart_dsstyle.ld",
        "-Wl,--gc-sections", *objects, "-L", gba / "lib", "-lgba", "-o", elf)
    run(objcopy, "-O", "binary", elf, rom)
    run(gbafix, rom, "-tDS STYLE TXT", "-cDSTV", "-m01")
    if not 0xc0 < rom.stat().st_size < 0x20000:
        raise RuntimeError("Unexpected text viewer size")


def build(sdk, flashable):
    source, images, output = ROOT / "source", ROOT / "images", ROOT / "build"
    output.mkdir(exist_ok=True)
    arm = sdk / "devkitARM/bin"
    gba = sdk / "libgba"

    def executable(folder, name):
        return folder / (name + (".exe" if os.name == "nt" else ""))

    gcc = executable(arm,"arm-none-eabi-gcc")
    nm = executable(arm,"arm-none-eabi-nm")
    objcopy = executable(arm,"arm-none-eabi-objcopy")
    includes = ["-iquote",source,"-iquote",source/"ff15","-iquote",images,"-I",gba/"include"]
    arch = ["-g","-mthumb","-mthumb-interwork","-mcpu=arm7tdmi"]

    def run(*args):
        subprocess.run(list(map(str,args)),cwd=ROOT,check=True)

    objects = []
    inputs = sorted(source.glob("*.c")) + sorted((source/"ff15").glob("*.c")) + sorted(source.glob("*.s"))
    for item in inputs:
        obj = output/(item.stem+".o")
        opts = ["-Wa,-I"+str(source)] if item.suffix == ".s" else [
            "-Wall","-Os" if item.name == "ezbluekernel.c" else "-O",
            "-mtune=arm7tdmi","-fomit-frame-pointer","-ffast-math"]
        run(gcc,*arch,*includes,*opts,"-c",item,"-o",obj)
        objects.append(obj)
    elf, rom = output/"ezbluekernel.elf", output/"ezbluekernel.gba"
    run(gcc,*arch,"-specs=gba.specs","-Wl,-Map,"+str(output/"ezbluekernel.map"),
        *sorted(objects),"-L",gba/"lib","-lmm","-lgba","-o",elf)
    regions = symbols(nm, elf)
    if regions["__eheap_start"] >= 0x0203f800 or regions["__iwram_overlay_end"] >= 0x03006f00:
        raise RuntimeError("Kernel leaves too little RAM for its boot and IRQ scratch")
    patch = symbols(nm, output / "gba_rts_patch.o")
    if not 0 < patch["DDR_OP_code_end"] - patch["DDR_OP_code"] <= 0x7f0:
        raise RuntimeError("Blue add-on DDR helper overlaps the return cookie")
    run(objcopy,"-O","binary",elf,rom)
    gbafix = executable(sdk/"tools/bin", "gbafix")
    run(gbafix,rom,"-tDSBLUE","-c0000","-m00")
    boot_obj, boot_rom = output/"blue-entry.o", output/"blue-entry.raw"
    run(executable(arm,"arm-none-eabi-as"),"-mcpu=arm7tdmi",ROOT/"tools/slot_boot_entry.s","-o",boot_obj)
    run(objcopy,"-O","binary","-j",".text",boot_obj,boot_rom)
    wrapper = symbols(nm, boot_obj)["Slot_ClearWorkRam"]
    payload = patch_entry(rom.read_bytes(),boot_rom.read_bytes(),wrapper)
    image = pack(payload,(ROOT/"image.bin").read_bytes())
    path = output/("ezbluekernel.bin" if flashable else "blue-upgrade-layout.test")
    build_text_viewer(run, gcc, objcopy, gbafix, arch, gba, output)
    path.write_bytes(image)
    print(f"{path}: {len(image)} bytes, SHA-256 {sha(image)}")


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    sdk = os.environ.get("DEVKITPRO","C:/devkitPro")
    if os.name == "nt" and len(sdk) >= 3 and sdk[0] == "/" and sdk[2] == "/":
        sdk = sdk[1] + ":" + sdk[2:]
    p.add_argument("--devkitpro",type=Path,default=Path(sdk))
    p.add_argument("--flashable",action="store_true",help="Name the Blue-only output ezbluekernel.bin")
    args = p.parse_args()
    build(args.devkitpro.resolve(),args.flashable)
