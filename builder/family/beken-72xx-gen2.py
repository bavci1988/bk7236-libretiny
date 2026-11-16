
from os.path import join

from platformio.platform.base import PlatformBase
from platformio.platform.board import PlatformBoardConfig
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
board: PlatformBoardConfig = env.BoardConfig()
platform: PlatformBase = env.PioPlatform()
queue = env.AddLibraryQueue("beken-72xx-gen2")
env.ConfigureFamily()

# Load sdkconfig.h into env
env.LoadConfig(join("$FAMILY_DIR", "base", "config", "sdkconfig.h"))

SOC = board.get("build.mcu")

# Flags
queue.AppendPublic(
    CCFLAGS=[
        "-mcpu=cortex-m33+nodsp",
        "-mfpu=fpv5-sp-d16",
        "-mfloat-abi=hard",
        "-mcmse",
        "-mthumb",
        "-fsigned-char",
        "-fno-strict-aliasing",
    ],
    CPPDEFINES=[
        # SDK options
        ("ARMINO_SOC", SOC),
        "BEKEN_PLATFORM",
        ("CONFIG_CMAKE", "1"),
        ("BK_MAC", "1"),
        ("BK_SUPPLICANT", "1"),
        "WIFI_BLE_COEXIST",
        ("CFG_LOG_LEVEL", "2"),
        ("CONFIG_RELEASE_VERSION", "1"),
        "_GNU_SOURCE",
    ],
    ASFLAGS=[
        "-mcpu=cortex-m33+nodsp",
        "-mfpu=fpv5-sp-d16",
        "-mfloat-abi=hard",
        "-mthumb",
        "-x",
        "assembler-with-cpp",
    ],
    LINKFLAGS=[
        "-mcpu=cortex-m33+nodsp",
        "-mfpu=fpv5-sp-d16",
        "-mfloat-abi=hard",
        "-mcmse",
        "-mthumb",
        "--specs=nano.specs",
        # startup code is provided by CMSIS (startup_bk7236.c)
        "-nostartfiles",
        # resolve these from ARMINO (bk_init, lwIP) before newlib pulls in its own copies
        "-Wl,-u,__stack_chk_guard",
        "-Wl,-u,__stack_chk_fail",
        "-Wl,-u,errno",
        # stdio wrappers (base/port/printf.c)
        "-Wl,-wrap,bk_printf",
    ],
)
queue.AppendPrivate(
    CCFLAGS=[
        "-Wno-address",
        "-Wno-address-of-packed-member",
        "-Wno-array-bounds",
        "-Wno-deprecated-declarations",
        "-Wno-format",
        "-Wno-unknown-pragmas",
        "-Wno-unused-function",
        "-Wno-unused-value",
    ],
    CFLAGS=[
        "-Wno-old-style-declaration",
    ],
)

# Sources - middleware/soc
queue.AddLibrary(
    name="armino_soc",
    base_dir=join("$SDK_DIR", "middleware/soc"),
    # include paths of all components, from the ARMINO build of a minimal project
    includes=[
        "+<$SDK_DIR/middleware/soc/bk7236/soc>",
        "+<$SDK_DIR/middleware/soc/bk7236/hal>",
        "+<$SDK_DIR/middleware/soc/bk7236>",
        "+<$SDK_DIR/include>",
        "+<$SDK_DIR/include/modules/securityip>",
        "+<$SDK_DIR/include/modules/freetype>",
        "+<$SDK_DIR/components/bk_common/include>",
        "+<$SDK_DIR/middleware/soc/common/soc/include>",
        "+<$SDK_DIR/middleware/soc/common/hal/include>",
        "+<$SDK_DIR/components/part_table>",
        "+<$SDK_DIR/middleware/boards/bk7236/partitions>",
        "+<$SDK_DIR/components/release>",
        "+<$SDK_DIR/components/bk_startup/freertos>",
        "+<$SDK_DIR/components/easy_flash>",
        "+<$SDK_DIR/components/base64>",
        "+<$SDK_DIR/components/bk_init/include>",
        "+<$SDK_DIR/components/os_source/freertos_v10/include>",
        "+<$SDK_DIR/components/os_source/freertos_v10/portable/GCC/ARM_CM33_NTZ/non_secure>",
        "+<$SDK_DIR/components/app>",
        "+<$SDK_DIR/components/bk_wifi/include>",
        "+<$SDK_DIR/components/bk_wifi/include/bk_private>",
        "+<$SDK_DIR/components/bk_netif/include>",
        "+<$SDK_DIR/components/bk_cli/include>",
        "+<$SDK_DIR/components/bk_cli/include/bk_private>",
        "+<$SDK_DIR/components/bk_cli>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/include/bk_private>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/hostapd>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/src/utils>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/src/ap>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/src/common>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/src/drivers>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/src>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/wpa_supplicant>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/bk_patch>",
        "+<$SDK_DIR/components/lwip_intf_v2_1/lwip-2.1.2/src/include/netif>",
        "+<$SDK_DIR/components/lwip_intf_v2_1/lwip-2.1.2/src/include/lwip>",
        "+<$SDK_DIR/components/lwip_intf_v2_1/lwip-2.1.2/src/include/lwip/priv>",
        "+<$SDK_DIR/components/utf8>",
        "+<$SDK_DIR/components/temp_detect>",
        "+<$SDK_DIR/middleware/driver>",
        "+<$SDK_DIR/middleware/driver/include>",
        "+<$SDK_DIR/middleware/driver/include/bk_private>",
        "+<$SDK_DIR/middleware/driver/include/bk_private/legacy>",
        "+<$SDK_DIR/middleware/driver/common>",
        "+<$SDK_DIR/middleware/driver/reset_reason>",
        "+<$SDK_DIR/middleware/driver/pwm>",
        "+<$SDK_DIR/middleware/driver/flash>",
        "+<$SDK_DIR/middleware/driver/uart>",
        "+<$SDK_DIR/middleware/driver/sys_ctrl>",
        "+<$SDK_DIR/middleware/driver/gpio>",
        "+<$SDK_DIR/middleware/driver/general_dma>",
        "+<$SDK_DIR/middleware/driver/icu>",
        "+<$SDK_DIR/middleware/driver/i2c>",
        "+<$SDK_DIR/middleware/driver/sdcard>",
        "+<$SDK_DIR/middleware/driver/saradc>",
        "+<$SDK_DIR/middleware/driver/pmu>",
        "+<$SDK_DIR/middleware/driver/mailbox>",
        "+<$SDK_DIR/middleware/driver/spinlock>",
        "+<$SDK_DIR/middleware/driver/touch>",
        "+<$SDK_DIR/middleware/driver/sbc>",
        "+<$SDK_DIR/middleware/driver/rtc>",
        "+<$SDK_DIR/middleware/driver/i2s>",
        "+<$SDK_DIR/middleware/driver/fft>",
        "+<$SDK_DIR/middleware/driver/chip_support>",
        "+<$SDK_DIR/middleware/compal/common_io/include>",
        "+<$SDK_DIR/middleware/driver/wdt>",
        "+<$SDK_DIR/middleware/driver/timer>",
        "+<$SDK_DIR/middleware/driver/scr>",
        "+<$SDK_DIR/middleware/driver/slcd>",
        "+<$SDK_DIR/middleware/driver/spi>",
        "+<$SDK_DIR/middleware/driver/jpeg_enc>",
        "+<$SDK_DIR/middleware/driver/bk7236>",
        "+<$SDK_DIR/middleware/driver/ckmn>",
        "+<$SDK_DIR/middleware/arch/cm33/include>",
        "+<$SDK_DIR/middleware/arch/cm33/os/freertos>",
        "+<$SDK_DIR/middleware/arch/cm33>",
        "+<$SDK_DIR/components/user_driver/include/bk_private>",
        "+<$SDK_DIR/components/bk_system/soc/bk7231n>",
        "+<$SDK_DIR/components/http>",
        "+<$SDK_DIR/components/adc_key>",
        "+<$SDK_DIR/components/key>",
        "+<$SDK_DIR/components/bk_websocket/include>",
        "+<$SDK_DIR/components/bk_rtos/include>",
        "+<$SDK_DIR/components/bk_rtos/freertos>",
        "+<$SDK_DIR/components/cmsis/CMSIS_5/CMSIS/Core/Include>",
        "+<$SDK_DIR/components/cmsis/CMSIS_5/Device/Beken/bk7236xx/Include>",
        "+<$SDK_DIR/components/cmsis/CMSIS_5/Device/Beken/bk7236xx/Include/Template>",
        "+<$SDK_DIR/components/cmsis/CMSIS_5/Device/Beken/armstar>",
        "+<$SDK_DIR/components/bk_ps/include>",
        "+<$SDK_DIR/components/lwip_intf_v2_1/lwip-2.1.2/src>",
        "+<$SDK_DIR/components/lwip_intf_v2_1/lwip-2.1.2/port>",
        "+<$SDK_DIR/components/lwip_intf_v2_1/lwip-2.1.2/src/include>",
        "+<$SDK_DIR/components/coredump/cm_backtrace>",
        "+<$SDK_DIR/components/coredump/cm_backtrace/Languages/en-US>",
        "+<$SDK_DIR/components/bk_phy/include>",
        "+<$SDK_DIR/components/wolfssl>",
        "+<$SDK_DIR/components/wolfssl/utils>",
        "+<$SDK_DIR/components/bk_adapter/include>",
        "+<$SDK_DIR/components/bk_adapter/include/bk_private>",
        "+<$SDK_DIR/components/bk_cli/uart_debug>",
        "+<$SDK_DIR/middleware/boards/bk7236/vnd_cal>",
        "+<$SDK_DIR/middleware/boards/bk7236/vnd_cal/include>",
        "+<$SDK_DIR/components/embeded_gcov/code>",
        "+<$SDK_DIR/components/bk_system/os/freertos>",
        "+<$SDK_DIR/components/unity/extras/fixture/src>",
        "+<$SDK_DIR/components/unity/src>",
        "+<$SDK_DIR/components/ota/include>",
        "+<$SDK_DIR/components/ota/include/bk_private>",
        "+<$SDK_DIR/components/bk_pm/include>",
        "+<$SDK_DIR/components/compress/include>",
        "+<$SDK_DIR/components/compress/lzma1900>",
        "+<$SDK_DIR/components/hmac_sha_256>",
        "+<$SDK_DIR/components/qcbor/inc>",
        "+<$SDK_DIR/components/qcbor/inc/qcbor>",
        "+<$SDK_DIR/middleware/boards/bk7236/vnd_flash>",
    ],
    srcs=[
        "+<bk7236/hal/adc_hal_debug.c>",
        "+<bk7236/hal/adc_ll.c>",
        "+<bk7236/hal/aon_pmu_hal.c>",
        "+<bk7236/hal/aon_pmu_hal_debug.c>",
        "+<bk7236/hal/aon_rtc_hal_debug.c>",
        "+<bk7236/hal/calendar_hal_debug.c>",
        "+<bk7236/hal/calendar_ll.c>",
        "+<bk7236/hal/ckmn_hal_debug.c>",
        "+<bk7236/hal/ckmn_ll.c>",
        "+<bk7236/hal/dma_hal_debug.c>",
        "+<bk7236/hal/dma_ll.c>",
        "+<bk7236/hal/efuse_hal_debug.c>",
        "+<bk7236/hal/efuse_ll.c>",
        "+<bk7236/hal/flash_hal_debug.c>",
        "+<bk7236/hal/flash_ll.c>",
        "+<bk7236/hal/gpio_hal_debug.c>",
        "+<bk7236/hal/gpio_ll.c>",
        "+<bk7236/hal/i2c_hal_debug.c>",
        "+<bk7236/hal/i2c_ll.c>",
        "+<bk7236/hal/i2s_hal.c>",
        "+<bk7236/hal/icu_hal_debug.c>",
        "+<bk7236/hal/icu_ll.c>",
        "+<bk7236/hal/jpeg_hal_debug.c>",
        "+<bk7236/hal/jpeg_ll.c>",
        "+<bk7236/hal/lin_hal_debug.c>",
        "+<bk7236/hal/lin_ll.c>",
        "+<bk7236/hal/psram_ll.c>",
        "+<bk7236/hal/pwm_hal_debug.c>",
        "+<bk7236/hal/pwm_ll.c>",
        "+<bk7236/hal/qspi_hal_debug.c>",
        "+<bk7236/hal/scr_hal.c>",
        "+<bk7236/hal/scr_hal_debug.c>",
        "+<bk7236/hal/sdio_hal_debug.c>",
        "+<bk7236/hal/sdmadc_hal_debug.c>",
        "+<bk7236/hal/spi_hal_debug.c>",
        "+<bk7236/hal/spi_ll.c>",
        "+<bk7236/hal/sys_clock_hal_debug.c>",
        "+<bk7236/hal/sys_hal.c>",
        "+<bk7236/hal/sys_hal_debug.c>",
        "+<bk7236/hal/sys_pm_hal.c>",
        "+<bk7236/hal/sys_pm_hal_ctrl.c>",
        "+<bk7236/hal/sys_pm_hal_debug.c>",
        "+<bk7236/hal/timer_hal.c>",
        "+<bk7236/hal/timer_hal_debug.c>",
        "+<bk7236/hal/timer_ll.c>",
        "+<bk7236/hal/trng_hal_debug.c>",
        "+<bk7236/hal/trng_ll.c>",
        "+<bk7236/hal/uart_hal_debug.c>",
        "+<bk7236/hal/uart_ll.c>",
        "+<bk7236/hal/wdt_hal_debug.c>",
        "+<bk7236/hal/wdt_ll.c>",
        "+<bk7236/hal/yuv_buf_hal_debug.c>",
        "+<bk7236/interrupts.c>",
        "+<bk7236/mpu_cfg.c>",
        "+<bk7236/stub.c>",
        "+<common/hal/adc_hal.c>",
        "+<common/hal/aon_rtc_hal_64bit.c>",
        "+<common/hal/ckmn_hal.c>",
        "+<common/hal/dma_hal.c>",
        "+<common/hal/efuse_hal.c>",
        "+<common/hal/flash_hal.c>",
        "+<common/hal/gpio_hal.c>",
        "+<common/hal/i2c_hal.c>",
        "+<common/hal/icu_hal.c>",
        "+<common/hal/jpeg_dec_hal.c>",
        "+<common/hal/jpeg_hal.c>",
        "+<common/hal/otp_hal.c>",
        "+<common/hal/pwm_hal_v1px.c>",
        "+<common/hal/qspi_hal.c>",
        "+<common/hal/sdio_hal.c>",
        "+<common/hal/sdmadc_hal.c>",
        "+<common/hal/spi_hal.c>",
        "+<common/hal/timer_hal.c>",
        "+<common/hal/trng_hal.c>",
        "+<common/hal/uart_hal.c>",
        "+<common/hal/wdt_hal.c>",
    ],
)

# Sources - components/base64
queue.AddLibrary(
    name="armino_base64",
    base_dir=join("$SDK_DIR", "components/base64"),
    srcs=[
        "+<base_64.c>",
    ],
)

# Sources - components/bk_rtos
queue.AddLibrary(
    name="armino_bk_rtos",
    base_dir=join("$SDK_DIR", "components/bk_rtos"),
    srcs=[
        "+<freertos/mem_arch.c>",
        "+<freertos/posix/freertos_impl/source/pthread_local_storage.c>",
        "+<freertos/str_arch.c>",
        "+<freertos/v10/rtos_debug.c>",
        "+<freertos/v10/rtos_pub.c>",
        "+<rtos_ext.c>",
    ],
)

# Sources - components/os_source
queue.AddLibrary(
    name="armino_os_source",
    base_dir=join("$SDK_DIR", "components/os_source"),
    srcs=[
        "+<freertos_v10/croutine.c>",
        "+<freertos_v10/event_groups.c>",
        "+<freertos_v10/list.c>",
        "+<freertos_v10/portable/Common/mpu_wrappers.c>",
        "+<freertos_v10/portable/GCC/ARM_CM33_NTZ/non_secure/port.c>",
        "+<freertos_v10/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c>",
        "+<freertos_v10/portable/MemMang/heap_4.c>",
        "+<freertos_v10/queue.c>",
        "+<freertos_v10/stream_buffer.c>",
        "+<freertos_v10/tasks.c>",
        "+<freertos_v10/timers.c>",
    ],
)

# Sources - components/coredump
queue.AddLibrary(
    name="armino_coredump",
    base_dir=join("$SDK_DIR", "components/coredump"),
    srcs=[
        "+<cm_backtrace/cm_backtrace.c>",
    ],
)

# Sources - components/cmsis
queue.AddLibrary(
    name="armino_cmsis",
    base_dir=join("$SDK_DIR", "components/cmsis"),
    srcs=[
        "+<CMSIS_5/Device/Beken/bk7236xx/Source/startup_bk7236.c>",
        "+<CMSIS_5/Device/Beken/bk7236xx/Source/system_bk7236xx.c>",
    ],
    options=dict(CPPDEFINES=[("__ARM_ARCH_8M_MAIN__", "1")]),
)

# Sources - middleware/arch
queue.AddLibrary(
    name="armino_arch",
    base_dir=join("$SDK_DIR", "middleware/arch"),
    srcs=[
        "+<cm33/arch_delay.c>",
        "+<cm33/arch_interrupt.c>",
        "+<cm33/arm.c>",
        "+<cm33/dwt.c>",
        "+<cm33/fpb.c>",
        "+<cm33/jump.c>",
        "+<cm33/mpu.c>",
        "+<cm33/os/freertos/cpu.c>",
        "+<cm33/stack_base.c>",
        "+<cm33/trap_base.c>",
    ],
    options=dict(CPPDEFINES=[("__ARM_ARCH_8M_MAIN__", "1")]),
)

# Sources - components/bk_log
queue.AddLibrary(
    name="armino_bk_log",
    base_dir=join("$SDK_DIR", "components/bk_log"),
    srcs=[
        "+<log.c>",
    ],
)

# Sources - components/bk_phy
queue.AddLibrary(
    name="armino_bk_phy",
    base_dir=join("$SDK_DIR", "components/bk_phy"),
    srcs=[
        "+<src/bk_phy_adapter.c>",
        "+<src/bk_phy_api.c>",
        "+<src/bk_rf_adapter.c>",
    ],
)

# Sources - components/bk_ate
queue.AddLibrary(
    name="armino_bk_ate",
    base_dir=join("$SDK_DIR", "components/bk_ate"),
    srcs=[
        "+<ate.c>",
    ],
)

# Sources - middleware/driver
queue.AddLibrary(
    name="armino_driver",
    base_dir=join("$SDK_DIR", "middleware/driver"),
    options=dict(
        CPPDEFINES=[
            # stdio is provided by the printf library (base/port/printf.c)
            ("__wrap_vsnprintf", "__armino_vsnprintf"),
            ("__wrap_snprintf", "__armino_snprintf"),
            ("__wrap_sprintf", "__armino_sprintf"),
            ("__wrap_puts", "__armino_puts"),
        ],
    ),
    srcs=[
        "+<audio/audio_ring_buff.c>",
        "+<bk7236/gpio_driver.c>",
        "+<bk7236/interrupt.c>",
        "+<bk7236/interrupt_statis.c>",
        "+<chip_support/chip_support.c>",
        "+<ckmn/ckmn_driver.c>",
        "+<common/dd.c>",
        "+<common/driver.c>",
        "+<common/drv_model.c>",
        "+<dsp/dsp.c>",
        "+<efuse/efuse_driver.c>",
        "+<flash/flash_bypass.c>",
        "+<flash/flash_driver.c>",
        "+<flash/flash_driver_ext.c>",
        "+<flash/flash_notify.c>",
        "+<flash/flash_partition.c>",
        "+<flash/flash_svr_test.c>",
        "+<general_dma/v2px/dma_driver.c>",
        "+<gpio/gpio_driver_base.c>",
        "+<hw_rotate/rott_driver.c>",
        "+<icu/icu_driver.c>",
        "+<icu/interrupt_base.c>",
        "+<jpeg_dec/jpeg_dec_driver.c>",
        "+<jpeg_enc/jpeg_driver.c>",
        "+<jpeg_enc/jpeg_statis.c>",
        "+<otp/otp_driver_v1_1.c>",
        "+<pmu/aon_pmu_driver.c>",
        "+<pwm/v1px/pwm_driver.c>",
        "+<pwr_clk/pwr_clk.c>",
        "+<pwr_clk/rosc_32k.c>",
        "+<pwr_clk/rosc_ppm.c>",
        "+<qspi/qspi_driver.c>",
        "+<qspi/qspi_flash.c>",
        "+<qspi/qspi_psram.c>",
        "+<qspi/qspi_statis.c>",
        "+<reset_reason/reset_reason.c>",
        "+<rtc/aon_rtc_driver_64bit.c>",
        "+<saradc/adc_driver.c>",
        "+<saradc/adc_statis.c>",
        "+<scr/scr_driver_v1_26.c>",
        "+<sdmadc/sdmadc_driver.c>",
        "+<security/hal_aes.c>",
        "+<security/hal_sha.c>",
        "+<security/security.c>",
        "+<spi/spi_driver.c>",
        "+<spi/spi_flash.c>",
        "+<spi/spi_statis.c>",
        "+<sys_ctrl/sys_audio_driver.c>",
        "+<sys_ctrl/sys_bt_driver.c>",
        "+<sys_ctrl/sys_clock_driver.c>",
        "+<sys_ctrl/sys_ctrl.c>",
        "+<sys_ctrl/sys_driver.c>",
        "+<sys_ctrl/sys_driver_common.c>",
        "+<sys_ctrl/sys_int_driver.c>",
        "+<sys_ctrl/sys_ps_driver.c>",
        "+<sys_ctrl/sys_psram_driver.c>",
        "+<sys_ctrl/sys_touch_driver.c>",
        "+<sys_ctrl/sys_video_driver.c>",
        "+<sys_ctrl/sys_wifi_driver.c>",
        "+<timer/timer_driver.c>",
        "+<touch/touch_driver_v1_1.c>",
        "+<trng/trng_driver.c>",
        "+<uart/printf.c>",
        "+<uart/uart_driver.c>",
        "+<uart/uart_statis.c>",
        "+<wdt/wdt_driver.c>",
    ],
)

# Sources - components/bk_adapter
queue.AddLibrary(
    name="armino_bk_adapter",
    base_dir=join("$SDK_DIR", "components/bk_adapter"),
    srcs=[
        "+<lib_adapter.c>",
    ],
)

# Sources - components/bk_wifi
queue.AddLibrary(
    name="armino_bk_wifi",
    base_dir=join("$SDK_DIR", "components/bk_wifi"),
    srcs=[
        "+<src/bk_wifi_adapter.c>",
        "+<src/bk_workqueue.c>",
        "+<src/hostapd_intf.c>",
        "+<src/lsig_monitor.c>",
        "+<src/net_param.c>",
        "+<src/pbuf.c>",
        "+<src/phy.c>",
        "+<src/rw_ieee80211.c>",
        "+<src/rw_msdu.c>",
        "+<src/rw_msg_rx.c>",
        "+<src/rw_msg_tx.c>",
        "+<src/rw_task.c>",
        "+<src/rw_tx_buffering.c>",
        "+<src/rwm_proto.c>",
        "+<src/rwnx_misc.c>",
        "+<src/rwnx_params.c>",
        "+<src/rwnx_rx.c>",
        "+<src/rwnx_td.c>",
        "+<src/rwnx_tx.c>",
        "+<src/rwnx_txq.c>",
        "+<src/rwnx_utils.c>",
        "+<src/sa_ap.c>",
        "+<src/sa_station.c>",
        "+<src/skbuff.c>",
        "+<src/soft_encrypt.c>",
        "+<src/wifi_api.c>",
        "+<src/wifi_config.c>",
        "+<src/wifi_frame.c>",
        "+<src/wifi_init.c>",
        "+<src/wifi_netif.c>",
        "+<src/wifi_v2.c>",
        "+<src/wifi_wpa_cmd.c>",
    ],
)

# Sources - middleware/boards
queue.AddLibrary(
    name="armino_boards",
    base_dir=join("$SDK_DIR", "middleware/boards"),
    srcs=[
        "+<bk7236/vnd_cal/vnd_cal.c>",
        "+<bk7236/vnd_flash/vnd_flash.c>",
    ],
)

# Sources - components/bk_init
queue.AddLibrary(
    name="armino_bk_init",
    base_dir=join("$SDK_DIR", "components/bk_init"),
    srcs=[
        "+<bk_init.c>",
        "+<components_init.c>",
    ],
)

# Sources - components/bk_startup
queue.AddLibrary(
    name="armino_bk_startup",
    base_dir=join("$SDK_DIR", "components/bk_startup"),
    srcs=[
        "+<freertos/rtos_init.c>",
        "+<system_main.c>",
    ],
    options=dict(
        # call lt_main() through the ARMINO user app entry (port/armino_main.c)
        CPPDEFINES=[("main", "lt_armino_main")],
    ),
)

# Sources - components/utf8
queue.AddLibrary(
    name="armino_utf8",
    base_dir=join("$SDK_DIR", "components/utf8"),
    srcs=[
        "+<conv_utf8.c>",
    ],
)

# Sources - components/temp_detect
queue.AddLibrary(
    name="armino_temp_detect",
    base_dir=join("$SDK_DIR", "components/temp_detect"),
    srcs=[
        "+<bk_sensor.c>",
        "+<temp_detect.c>",
        "+<volt_detect.c>",
    ],
)

# Sources - components/bk_system
queue.AddLibrary(
    name="armino_bk_system",
    base_dir=join("$SDK_DIR", "components/bk_system"),
    srcs=[
        "+<delay.c>",
        "+<mac.c>",
        "+<os/freertos/tick.c>",
        "+<printf.c>",
        "+<printf_base.c>",
        "+<reboot.c>",
        "+<tick_base.c>",
    ],
)

# Sources - components/unity
queue.AddLibrary(
    name="armino_unity",
    base_dir=join("$SDK_DIR", "components/unity"),
    srcs=[
        "+<extras/fixture/src/unity_fixture.c>",
        "+<src/unity.c>",
    ],
)

# Sources - components/bk_cli
queue.AddLibrary(
    name="armino_bk_cli",
    base_dir=join("$SDK_DIR", "components/bk_cli"),
    srcs=[
        "+<cli_ckmn.c>",
        "+<cli_main.c>",
        "+<cli_scr.c>",
        "+<shell_task.c>",
        "+<shell_uart.c>",
    ],
)

# Sources - components/bk_event
queue.AddLibrary(
    name="armino_bk_event",
    base_dir=join("$SDK_DIR", "components/bk_event"),
    srcs=[
        "+<event.c>",
    ],
)

# Sources - components/bk_common
queue.AddLibrary(
    name="armino_bk_common",
    base_dir=join("$SDK_DIR", "components/bk_common"),
    srcs=[
        "+<bk_feature.c>",
    ],
)

# Sources - components/bk_pm
queue.AddLibrary(
    name="armino_bk_pm",
    base_dir=join("$SDK_DIR", "components/bk_pm"),
    srcs=[
        "+<low_vol.c>",
        "+<pm.c>",
        "+<pm_misc.c>",
        "+<pm_model.c>",
    ],
)

# Sources - components/compress
queue.AddLibrary(
    name="armino_compress",
    base_dir=join("$SDK_DIR", "components/compress"),
    srcs=[
        "+<decompress.c>",
        "+<lzma.c>",
        "+<lzma1900/LzmaDec.c>",
    ],
)

# Sources - components/demos
queue.AddLibrary(
    name="armino_demos",
    base_dir=join("$SDK_DIR", "components/demos"),
    srcs=[
        "+<peripheral/touch/touch_demo.c>",
    ],
)

# Sources - components/hmac_sha_256
queue.AddLibrary(
    name="armino_hmac_sha_256",
    base_dir=join("$SDK_DIR", "components/hmac_sha_256"),
    srcs=[
        "+<pbkdf2_sha256.c>",
    ],
)

# Sources - components/qcbor
queue.AddLibrary(
    name="armino_qcbor",
    base_dir=join("$SDK_DIR", "components/qcbor"),
    srcs=[
        "+<src/UsefulBuf.c>",
        "+<src/ieee754.c>",
        "+<src/qcbor_decode.c>",
        "+<src/qcbor_encode.c>",
        "+<src/qcbor_err_to_str.c>",
    ],
)

# Sources - components/security
queue.AddLibrary(
    name="armino_security",
    base_dir=join("$SDK_DIR", "components/security"),
    srcs=[
        "+<security_func.c>",
    ],
)

# Sources - middleware/soc
queue.AddLibrary(
    name="armino_soc",
    base_dir=join("$SDK_DIR", "middleware/soc"),
    srcs=[
        "+<bk7236/hal/adc_hal_debug.c>",
        "+<bk7236/hal/adc_ll.c>",
        "+<bk7236/hal/aon_pmu_hal.c>",
        "+<bk7236/hal/aon_pmu_hal_debug.c>",
        "+<bk7236/hal/aon_rtc_hal_debug.c>",
        "+<bk7236/hal/calendar_hal_debug.c>",
        "+<bk7236/hal/calendar_ll.c>",
        "+<bk7236/hal/ckmn_hal_debug.c>",
        "+<bk7236/hal/ckmn_ll.c>",
        "+<bk7236/hal/dma_hal_debug.c>",
        "+<bk7236/hal/dma_ll.c>",
        "+<bk7236/hal/efuse_hal_debug.c>",
        "+<bk7236/hal/efuse_ll.c>",
        "+<bk7236/hal/flash_hal_debug.c>",
        "+<bk7236/hal/flash_ll.c>",
        "+<bk7236/hal/gpio_hal_debug.c>",
        "+<bk7236/hal/gpio_ll.c>",
        "+<bk7236/hal/i2c_hal_debug.c>",
        "+<bk7236/hal/i2c_ll.c>",
        "+<bk7236/hal/i2s_hal.c>",
        "+<bk7236/hal/icu_hal_debug.c>",
        "+<bk7236/hal/icu_ll.c>",
        "+<bk7236/hal/jpeg_hal_debug.c>",
        "+<bk7236/hal/jpeg_ll.c>",
        "+<bk7236/hal/lin_hal_debug.c>",
        "+<bk7236/hal/lin_ll.c>",
        "+<bk7236/hal/psram_ll.c>",
        "+<bk7236/hal/pwm_hal_debug.c>",
        "+<bk7236/hal/pwm_ll.c>",
        "+<bk7236/hal/qspi_hal_debug.c>",
        "+<bk7236/hal/scr_hal.c>",
        "+<bk7236/hal/scr_hal_debug.c>",
        "+<bk7236/hal/sdio_hal_debug.c>",
        "+<bk7236/hal/sdmadc_hal_debug.c>",
        "+<bk7236/hal/spi_hal_debug.c>",
        "+<bk7236/hal/spi_ll.c>",
        "+<bk7236/hal/sys_clock_hal_debug.c>",
        "+<bk7236/hal/sys_hal.c>",
        "+<bk7236/hal/sys_hal_debug.c>",
        "+<bk7236/hal/sys_pm_hal.c>",
        "+<bk7236/hal/sys_pm_hal_ctrl.c>",
        "+<bk7236/hal/sys_pm_hal_debug.c>",
        "+<bk7236/hal/timer_hal.c>",
        "+<bk7236/hal/timer_hal_debug.c>",
        "+<bk7236/hal/timer_ll.c>",
        "+<bk7236/hal/trng_hal_debug.c>",
        "+<bk7236/hal/trng_ll.c>",
        "+<bk7236/hal/uart_hal_debug.c>",
        "+<bk7236/hal/uart_ll.c>",
        "+<bk7236/hal/wdt_hal_debug.c>",
        "+<bk7236/hal/wdt_ll.c>",
        "+<bk7236/hal/yuv_buf_hal_debug.c>",
        "+<bk7236/interrupts.c>",
        "+<bk7236/mpu_cfg.c>",
        "+<bk7236/stub.c>",
        "+<common/hal/adc_hal.c>",
        "+<common/hal/aon_rtc_hal_64bit.c>",
        "+<common/hal/ckmn_hal.c>",
        "+<common/hal/dma_hal.c>",
        "+<common/hal/efuse_hal.c>",
        "+<common/hal/flash_hal.c>",
        "+<common/hal/gpio_hal.c>",
        "+<common/hal/i2c_hal.c>",
        "+<common/hal/icu_hal.c>",
        "+<common/hal/jpeg_dec_hal.c>",
        "+<common/hal/jpeg_hal.c>",
        "+<common/hal/otp_hal.c>",
        "+<common/hal/pwm_hal_v1px.c>",
        "+<common/hal/qspi_hal.c>",
        "+<common/hal/sdio_hal.c>",
        "+<common/hal/sdmadc_hal.c>",
        "+<common/hal/spi_hal.c>",
        "+<common/hal/timer_hal.c>",
        "+<common/hal/trng_hal.c>",
        "+<common/hal/uart_hal.c>",
        "+<common/hal/wdt_hal.c>",
    ],
)

# Sources - components/wolfssl
queue.AddLibrary(
    name="armino_wolfssl",
    base_dir=join("$SDK_DIR", "components/wolfssl"),
    srcs=[
        "+<wolfcrypt/src/ecc.c>",
        "+<wolfcrypt/src/hmac.c>",
        "+<wolfcrypt/src/md5.c>",
        "+<wolfcrypt/src/memory.c>",
        "+<wolfcrypt/src/random.c>",
        "+<wolfcrypt/src/sha.c>",
        "+<wolfcrypt/src/sha256.c>",
        "+<wolfcrypt/src/sha512.c>",
        "+<wolfcrypt/src/tfm.c>",
        "+<wolfcrypt/src/wolfmath.c>",
    ],
)

# Sources - components/wpa_supplicant-2.10
queue.AddLibrary(
    name="armino_wpa_supplicant_2_10",
    base_dir=join("$SDK_DIR", "components/wpa_supplicant-2.10"),
    options=dict(
        # own headers first, like in ARMINO (list.h, ip_addr.h, cli.h, config.h collide)
        CCFLAGS=[
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/include/bk_private",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/hostapd",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/src/utils",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/src/ap",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/src/common",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/src/drivers",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/src",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/wpa_supplicant",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/bk_patch",
        ],
    ),
    srcs=[
        "+<bk_patch/ddrv.c>",
        "+<bk_patch/fake_socket.c>",
        "+<bk_patch/signal.c>",
        "+<bk_patch/sk_intf.c>",
        "+<hostapd/main_none.c>",
        "+<src/ap/ap_config.c>",
        "+<src/ap/ap_drv_ops.c>",
        "+<src/ap/ap_list.c>",
        "+<src/ap/ap_mlme.c>",
        "+<src/ap/beacon.c>",
        "+<src/ap/dfs.c>",
        "+<src/ap/drv_callbacks.c>",
        "+<src/ap/hostapd.c>",
        "+<src/ap/hw_features.c>",
        "+<src/ap/ieee802_11.c>",
        "+<src/ap/ieee802_11_auth.c>",
        "+<src/ap/ieee802_11_he.c>",
        "+<src/ap/ieee802_11_ht.c>",
        "+<src/ap/ieee802_11_shared.c>",
        "+<src/ap/ieee802_11_vht.c>",
        "+<src/ap/ieee802_1x.c>",
        "+<src/ap/pmksa_cache_auth.c>",
        "+<src/ap/sta_info.c>",
        "+<src/ap/tkip_countermeasures.c>",
        "+<src/ap/utils.c>",
        "+<src/ap/wmm.c>",
        "+<src/ap/wpa_auth.c>",
        "+<src/ap/wpa_auth_glue.c>",
        "+<src/ap/wpa_auth_ie.c>",
        "+<src/common/dragonfly.c>",
        "+<src/common/hw_features_common.c>",
        "+<src/common/ieee802_11_common.c>",
        "+<src/common/ocv.c>",
        "+<src/common/sae.c>",
        "+<src/common/wpa_common.c>",
        "+<src/common/wpa_psk_cache.c>",
        "+<src/crypto/aes-cbc.c>",
        "+<src/crypto/aes-ctr.c>",
        "+<src/crypto/aes-internal-dec.c>",
        "+<src/crypto/aes-internal-enc.c>",
        "+<src/crypto/aes-internal.c>",
        "+<src/crypto/aes-omac1.c>",
        "+<src/crypto/aes-siv.c>",
        "+<src/crypto/aes-unwrap.c>",
        "+<src/crypto/aes-wrap.c>",
        "+<src/crypto/crypto_internal-modexp.c>",
        "+<src/crypto/crypto_wolfssl.c>",
        "+<src/crypto/dh_group5.c>",
        "+<src/crypto/dh_groups.c>",
        "+<src/crypto/md5-internal.c>",
        "+<src/crypto/md5.c>",
        "+<src/crypto/rc4.c>",
        "+<src/crypto/sha1-internal.c>",
        "+<src/crypto/sha1-pbkdf2.c>",
        "+<src/crypto/sha1-prf.c>",
        "+<src/crypto/sha1.c>",
        "+<src/crypto/sha256-kdf.c>",
        "+<src/crypto/sha256-prf.c>",
        "+<src/crypto/sha384-internal.c>",
        "+<src/crypto/sha384-kdf.c>",
        "+<src/crypto/sha384-prf.c>",
        "+<src/crypto/sha384.c>",
        "+<src/crypto/sha512-internal.c>",
        "+<src/crypto/sha512-kdf.c>",
        "+<src/crypto/sha512-prf.c>",
        "+<src/crypto/sha512.c>",
        "+<src/crypto/tls_none.c>",
        "+<src/drivers/driver_beken.c>",
        "+<src/drivers/driver_common.c>",
        "+<src/drivers/drivers.c>",
        "+<src/l2_packet/l2_packet_none.c>",
        "+<src/rsn_supp/pmksa_cache.c>",
        "+<src/rsn_supp/wpa.c>",
        "+<src/rsn_supp/wpa_ie.c>",
        "+<src/utils/common.c>",
        "+<src/utils/crc32.c>",
        "+<src/utils/eloop.c>",
        "+<src/utils/os_none.c>",
        "+<src/utils/wpa_debug.c>",
        "+<src/utils/wpabuf.c>",
        "+<wpa_supplicant/bss.c>",
        "+<wpa_supplicant/bssid_ignore.c>",
        "+<wpa_supplicant/config.c>",
        "+<wpa_supplicant/config_none.c>",
        "+<wpa_supplicant/ctrl_iface.c>",
        "+<wpa_supplicant/events.c>",
        "+<wpa_supplicant/main_supplicant.c>",
        "+<wpa_supplicant/notify.c>",
        "+<wpa_supplicant/op_classes.c>",
        "+<wpa_supplicant/sme.c>",
        "+<wpa_supplicant/wmm_ac.c>",
        "+<wpa_supplicant/wnm_sta.c>",
        "+<wpa_supplicant/wpa_scan.c>",
        "+<wpa_supplicant/wpa_supplicant.c>",
        "+<wpa_supplicant/wpas_glue.c>",
    ],
)

# Sources - components/lwip_intf_v2_1
queue.AddLibrary(
    name="armino_lwip_intf_v2_1",
    base_dir=join("$SDK_DIR", "components/lwip_intf_v2_1"),
    srcs=[
        "+<dhcpd/dhcp-server-main.c>",
        "+<dhcpd/dhcp-server.c>",
        "+<lwip-2.1.2/port/net.c>",
        "+<lwip-2.1.2/port/sys_arch.c>",
        "+<lwip-2.1.2/port/wlanif.c>",
        "+<lwip-2.1.2/src/api/api_lib.c>",
        "+<lwip-2.1.2/src/api/api_msg.c>",
        "+<lwip-2.1.2/src/api/err.c>",
        "+<lwip-2.1.2/src/api/netbuf.c>",
        "+<lwip-2.1.2/src/api/netdb.c>",
        "+<lwip-2.1.2/src/api/netifapi.c>",
        "+<lwip-2.1.2/src/api/sockets.c>",
        "+<lwip-2.1.2/src/api/tcpip.c>",
        "+<lwip-2.1.2/src/apps/mdns/mdns.c>",
        "+<lwip-2.1.2/src/apps/ping/ping.c>",
        "+<lwip-2.1.2/src/core/debug.c>",
        "+<lwip-2.1.2/src/core/def.c>",
        "+<lwip-2.1.2/src/core/dns.c>",
        "+<lwip-2.1.2/src/core/inet_chksum.c>",
        "+<lwip-2.1.2/src/core/init.c>",
        "+<lwip-2.1.2/src/core/ip.c>",
        "+<lwip-2.1.2/src/core/ipv4/autoip.c>",
        "+<lwip-2.1.2/src/core/ipv4/dhcp.c>",
        "+<lwip-2.1.2/src/core/ipv4/etharp.c>",
        "+<lwip-2.1.2/src/core/ipv4/icmp.c>",
        "+<lwip-2.1.2/src/core/ipv4/igmp.c>",
        "+<lwip-2.1.2/src/core/ipv4/ip4.c>",
        "+<lwip-2.1.2/src/core/ipv4/ip4_addr.c>",
        "+<lwip-2.1.2/src/core/ipv4/ip4_frag.c>",
        "+<lwip-2.1.2/src/core/ipv4/ip4_napt.c>",
        "+<lwip-2.1.2/src/core/ipv6/dhcp6.c>",
        "+<lwip-2.1.2/src/core/ipv6/ethip6.c>",
        "+<lwip-2.1.2/src/core/ipv6/icmp6.c>",
        "+<lwip-2.1.2/src/core/ipv6/inet6.c>",
        "+<lwip-2.1.2/src/core/ipv6/ip6.c>",
        "+<lwip-2.1.2/src/core/ipv6/ip6_addr.c>",
        "+<lwip-2.1.2/src/core/ipv6/ip6_frag.c>",
        "+<lwip-2.1.2/src/core/ipv6/mld6.c>",
        "+<lwip-2.1.2/src/core/ipv6/nd6.c>",
        "+<lwip-2.1.2/src/core/mem.c>",
        "+<lwip-2.1.2/src/core/memp.c>",
        "+<lwip-2.1.2/src/core/netif.c>",
        "+<lwip-2.1.2/src/core/pbuf.c>",
        "+<lwip-2.1.2/src/core/raw.c>",
        "+<lwip-2.1.2/src/core/stats.c>",
        "+<lwip-2.1.2/src/core/sys.c>",
        "+<lwip-2.1.2/src/core/tcp.c>",
        "+<lwip-2.1.2/src/core/tcp_in.c>",
        "+<lwip-2.1.2/src/core/tcp_out.c>",
        "+<lwip-2.1.2/src/core/timeouts.c>",
        "+<lwip-2.1.2/src/core/udp.c>",
        "+<lwip-2.1.2/src/netif/ethernet.c>",
    ],
)

# Sources - components/bk_netif
queue.AddLibrary(
    name="armino_bk_netif",
    base_dir=join("$SDK_DIR", "components/bk_netif"),
    srcs=[
        "+<bk_netif.c>",
    ],
)

# Libs & linker config
queue.AppendPublic(
    LIBPATH=[
        join("$SDK_DIR", "components", "bk_libs", SOC, "libs"),
    ],
    LIBS=[
        "bk_phy",
        "bk_phy_info",
        "ble_wifi_exchange",
        "usb",
        "wifi",
        "gcc",
        "g",
        "c",
        "m",
        "nosys",
    ],
)

# Misc options
env.Replace(
    # sections stored in flash (.itcm/.dtcm/.iram/.data are copied to RAM at startup)
    SIZEPROGREGEXP=r"^(?:\.vectors|\.text|\.ARM\.exidx|\.init_array|\.copy\.table|\.zero\.table|\.itcm|\.dtcm|\.iram|\.data)\s+([0-9]+).*",
    # .ramcode reserves the RAM used by .iram
    SIZEDATAREGEXP=r"^(?:\.ramcode|\.data|\.uninitialized|\.bss)\s+([0-9]+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD="$SIZETOOL -B -d $SOURCES",
)
# Generate linker scripts with correct flash offsets
env.GenerateLinkerScript(board, board.get("build.ldscript"))

# Build all libraries
queue.BuildLibraries()

# Main firmware outputs and actions
image_app_crc = "${BUILD_DIR}/image_${MCULC}_app.${FLASH_APP_OFFSET}.crc"
env.Replace(
    # linker command (CRC + packaging)
    LINK='${LTCHIPTOOL} link2bin ${BOARD_JSON} "" ""',
    # UF2OTA input list
    UF2OTA=[
        # app binary image (crc) for flasher
        f"{image_app_crc}=flasher:app",
    ],
)
