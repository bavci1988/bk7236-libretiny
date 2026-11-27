from os.path import join
from shutil import copyfile

from platformio.platform.board import PlatformBoardConfig
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
board: PlatformBoardConfig = env.BoardConfig()
queue = env.AddLibraryQueue("xradio-xr806")
env.ConfigureFamily()

# Load prj_config.h into env
env.LoadConfig(join("$FAMILY_DIR", "base", "config", "prj_config.h"))

CPU = [
    "-mcpu=cortex-m33",
    "-march=armv8-m.main+dsp",
    "-mfpu=fpv5-sp-d16",
    "-mfloat-abi=softfp",
    "-mcmse",
    "-mthumb",
]

# Power design of the board: "ldo" (e.g. WXU) or "dcdc" (an inductor at VDD_LX).
# The SDK and the bootloader stop at boot if this does not match the hardware.
power = board.get("build.xr806_power", "ldo")
if power not in ("ldo", "dcdc"):
    raise ValueError(f"Unknown XR806 power design '{power}', expected 'ldo' or 'dcdc'")
boot_bin = "boot_40M.bin"
if power == "dcdc":
    queue.AppendPublic(CPPDEFINES=[("CONFIG_PWR_INTERNAL_DCDC", "1")])
    boot_bin = "boot_40M_dcdc.bin"

# Flags
queue.AppendPublic(
    CCFLAGS=[
        *CPU,
        "-fno-common",
    ],
    CPPDEFINES=[
        # chip.mk
        ("CONFIG_CHIP_ARCH_VER", "3"),
        "CONFIG_ARCH_APP_CORE",
        "CONFIG_CPU_CM33F",
        ("PLATFORM_XR806", "1"),
        # config.mk
        ("CONFIG_RAM_START", "0x00201000"),
        ("CONFIG_PSRAM_START", "0x01400000"),
        ("MBEDTLS_CONFIG_FILE", r"\"mbedtls_config.h\""),
    ],
    CPPPATH=[
        # SDK public headers
        join("$SDK_DIR", "include"),
        join("$SDK_DIR", "include", "driver", "cmsis"),
        join("$SDK_DIR", "include", "net"),
        join("$SDK_DIR", "include", "net", "mbedtls-2.16.0"),
        join("$SDK_DIR", "include", "net", "mbedtls-2.16.0", "mbedtls", "configs"),
        join("$SDK_DIR", "include", "kernel", "FreeRTOS", "FreeRTOSv10.2.1"),
        join(
            "$SDK_DIR",
            "include",
            "kernel",
            "FreeRTOS",
            "FreeRTOSv10.2.1",
            "portable",
            "GCC",
            "ARM_CM33_NTZ",
        ),
        join(
            "$SDK_DIR",
            "include",
            "kernel",
            "FreeRTOS",
            "FreeRTOSv10.2.1",
            "portable",
            "GCC",
            "ARM_CM33_NTZ",
            "non_secure",
        ),
        join("$SDK_DIR", "project"),
    ],
    ASFLAGS=[
        *CPU,
        "-x",
        "assembler-with-cpp",
        "-c",
        # gcc.mk AS_SYMBOLS
        "-DCONFIG_CHIP_ARCH_VER=3",
        "-DCONFIG_CPU_CM33F",
        "-include",
        join("$SDK_DIR", "include", "generated", "autoconf.h"),
    ],
    LINKFLAGS=[
        *CPU,
        "--specs=nano.specs",
        # startup.s calls _start (base/port/xr806_main.c) instead of the C library's crt0
        "-nostartfiles",
        "-Wl,-wrap,exit",
        "-Wl,-wrap,gettimeofday",
        "-Wl,-wrap,settimeofday",
        "-Wl,-wrap,time",
        "-Wl,-wrap,memcpy",
        "-Wl,-wrap,memset",
        "-Wl,-wrap,memmove",
        # the "chip", "xrsys" and "rom" libraries override weak symbols of each other;
        # "xrc" has the heap wrappers that the ROM calls (wrap_malloc.c)
        "-Wl,--whole-archive",
        "-lchip",
        "-lxrsys",
        "-lrom",
        "-lxrc",
        "-Wl,--no-whole-archive",
    ],
)
queue.AppendPrivate(
    CCFLAGS=[
        # only the SDK uses include/libc, which redefines int32_t as int
        "-I" + join("$SDK_DIR", "include", "libc"),
        "-include",
        "sys/types.h",
        "-include",
        "xr_config.h",
        "-Wno-cpp",
        "-Wno-pointer-arith",
        "-Wno-unused-function",
        "-Wno-unused-variable",
        "-Wno-unused-but-set-variable",
        "-Wno-format-truncation",
    ],
    CPPDEFINES=[
        # base/config/prj_config.h
        ("FLASH_SYSINFO_OFFSET", "${FLASH_SYSINFO_OFFSET}"),
    ],
    CFLAGS=[
        "-Wno-implicit-function-declaration",
        "-Wno-incompatible-pointer-types",
        "-Wno-int-conversion",
        "-Wno-discarded-qualifiers",
    ],
)

# Sources - SDK drivers and system
queue.AddLibrary(
    name="chip",
    base_dir=join("$SDK_DIR", "src", "driver", "chip"),
    srcs=[
        "+<*.c>",
        "+<*/*.c>",
        "-<psram/*.c>",
        "-<sdmmc/*.c>",
    ],
)
queue.AddLibrary(
    name="xrsys",
    base_dir=join("$SDK_DIR", "src", "sys"),
    srcs=[
        "+<**/*.c>",
        "+<**/*.s>",
    ],
)
queue.AddLibrary(
    name="rom",
    base_dir=join("$SDK_DIR", "src", "rom"),
    srcs=[
        "+<rom_core.c>",
    ],
)
queue.AddLibrary(
    name="xrc",
    base_dir=join("$SDK_DIR", "src", "libc"),
    srcs=[
        # stdio goes to the LibreTiny printf library instead of wrap_stdio.c
        "+<wrap_malloc.c>",
        "+<wrap_time.c>",
    ],
)
queue.AddLibrary(
    name="os",
    base_dir=join("$SDK_DIR", "src", "kernel"),
    srcs=[
        # the FreeRTOS kernel is in ROM; only the patched files are built
        "+<FreeRTOS/FreeRTOSv10.2.1/Patch/*.c>",
        "+<os/FreeRTOS/*.c>",
    ],
)
for name in ["image", "pm", "efpg", "sdd", "util", "debug"]:
    queue.AddLibrary(
        name=name,
        base_dir=join("$SDK_DIR", "src", name),
        srcs=[
            "+<*.c>",
            "+<*.s>",
        ],
    )
queue.AddLibrary(
    name="ota",
    base_dir=join("$SDK_DIR", "src", "ota"),
    srcs=[
        "+<*.c>",
        "-<ota_http.c>",
    ],
)
queue.AddLibrary(
    name="mbuf",
    base_dir=join("$SDK_DIR", "src", "wlan"),
    srcs=[
        "+<*.c>",
        "+<mbuf/*.c>",
    ],
)

# Sources - project framework (startup, board, platform and network init)
queue.AddLibrary(
    name="xr806_project",
    base_dir=join("$SDK_DIR", "project", "common"),
    srcs=[
        "+<startup/gcc/*.c>",
        "+<startup/gcc/*.s>",
        "+<board/board.c>",
        "+<board/board_common.c>",
        "+<board/xr806_wxu/*.c>",
        "+<framework/platform_init.c>",
        "+<framework/net_ctrl.c>",
        "+<framework/net_sys.c>",
        "+<framework/sysinfo.c>",
        "+<framework/sys_monitor.c>",
        "+<framework/sys_ctrl/*.c>",
    ],
    includes=[
        "+<.>",
        "+<board/xr806_wxu>",
        "+<framework>",
    ],
    options=dict(
        # project/project.mk
        CCFLAGS=["-include", "common/prj_conf_opt.h"],
    ),
)

# Sources - DHCP server for the access point
queue.AddLibrary(
    name="udhcpd",
    base_dir=join("$SDK_DIR", "src", "net", "udhcp-0.9.8"),
    srcs=[
        "+<usr_dhcpd.c>",
        "+<arpping.c>",
        "+<files.c>",
        "+<leases.c>",
        "+<serverpacket.c>",
        "+<options.c>",
        "+<socket.c>",
        "+<packet.c>",
        "+<dhcp_time.c>",
        "+<dns_server.c>",
    ],
    options=dict(
        CPPDEFINES=[
            ("VERSION", r"\"0.9.8\""),
            "XR_DHCPD",
            "DHCPD_LWIP",
            "DHCPD_HEAP_REPLACE_STACK",
            "DHCPD_USRCFG",
            "DHCPD_FREERTOS",
            "DHCPD_ICMPPING",
            "DHCPD_LOW_LEVEL",
            "DHCPD_USE_DEFAULT_INIT",
            "DHCPD_UPGRADE_STA_MAC",
            "DHCPD_TIMEALT",
        ],
    ),
)

# Sources - lwIP
queue.AddExternalLibrary("lwip", port="xr806")

# Sources - mbedTLS
queue.AddLibrary(
    name="mbedtls",
    base_dir=join("$SDK_DIR", "src", "net", "mbedtls-2.16.0"),
    srcs=[
        "+<library/*.c>",
    ],
)

# Libs & linker config
queue.AppendPublic(
    LIBPATH=[
        join("$SDK_DIR", "lib"),
        join("$SDK_DIR", "lib", "xradio_v3"),
    ],
    LIBS=[
        "wlan",
        "wpas_hostapd",
        "net80211",
        "xrwireless",
        "net80211",
        "m",
        "c",
        "nosys",
        "gcc",
    ],
)

# PHY_RF_DPD_RecipFunc() of the PHY library loops forever for an input of 0;
# base/fixups/phy_rf_dpd.c replaces it in a copy where the symbol is weak
phy_lib = env.Command(
    join("$BUILD_DIR", "libxrwireless_phy_fixed.a"),
    join("$SDK_DIR", "lib", "xradio_v3", "libxrwireless_phy.a"),
    "$OBJCOPY --weaken-symbol=PHY_RF_DPD_RecipFunc $SOURCE $TARGET",
)
queue.AppendPublic(
    LIBS=[phy_lib],
    LINKFLAGS=["-Wl,-u,PHY_RF_DPD_RecipFunc"],
)

# Build all libraries
queue.BuildLibraries()

# Copy the SDK image parts for ltchiptool (bootloader, WLAN firmware, radio config)
for src, dst in [
    (join("bin", "xradio_v3", "boot", "xr806", boot_bin), "boot.bin"),
    (join("bin", "xradio_v3", "wlan_bl.bin"), "wlan_bl.bin"),
    (join("bin", "xradio_v3", "wlan_fw.bin"), "wlan_fw.bin"),
    (join("bin", "xradio_v3", "sys_sdd_40M.bin"), "sys_sdd.bin"),
]:
    copyfile(join(env.subst("$SDK_DIR"), src), join(env.subst("$BUILD_DIR"), dst))

# Main firmware outputs and actions
image_ota = "${BUILD_DIR}/image_ota.${FLASH_DOWNLOAD_OFFSET}.bin"
image_boot = "${BUILD_DIR}/image_boot.${FLASH_BOOT_OFFSET}.bin"
image_app = "${BUILD_DIR}/image_app.${FLASH_APP_OFFSET}.bin"
env.Replace(
    # linker command
    LINK='${LTCHIPTOOL} link2bin ${BOARD_JSON} "" ""',
    # UF2OTA input list
    UF2OTA=[
        # XZ-compressed app for the download partition; the bootloader unpacks it
        f"{image_ota}=device:download",
        # binary images for flasher
        f"{image_boot}=flasher:boot",
        f"{image_app}=flasher:app",
    ],
)
