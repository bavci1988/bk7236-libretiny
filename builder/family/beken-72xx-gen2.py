
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
        ("BK_MAC", "1"),
        ("BK_SUPPLICANT", "1"),
        ("CFG_LOG_LEVEL", "2"),
        ("CONFIG_RELEASE_VERSION", "1"),
        "_GNU_SOURCE",
        ("MBEDTLS_CONFIG_FILE", r"\"tls_config.h\""),
        # bk_idk release shown at boot (normally the git hash of the SDK and its daily build ID)
        ("ARMINO_VER", r"\"v2.2.1.25\""),
        ("BK_VERIFY_ID", r"\"D18804\""),
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
        # startup code is provided by CMSIS (startup_bk7236xx.c)
        "-nostartfiles",
        # resolve these from ARMINO (bk_init) before newlib pulls in its own copies
        "-Wl,-u,__stack_chk_guard",
        "-Wl,-u,__stack_chk_fail",
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
        "+<$SDK_DIR/components/bk_common/include>",
        "+<$SDK_DIR/middleware/soc/common/soc/include>",
        "+<$SDK_DIR/middleware/soc/common/hal/include>",
        "+<$SDK_DIR/components/release>",
        "+<$SDK_DIR/components/bk_startup/freertos>",
        "+<$SDK_DIR/components/base64>",
        "+<$SDK_DIR/components/bk_init/include>",
        "+<$SDK_DIR/components/os_source/freertos_v10/include>",
        "+<$SDK_DIR/components/os_source/freertos_v10/portable/GCC/ARM_CM33_NTZ/non_secure>",
        "+<$SDK_DIR/components/bk_wifi/include>",
        "+<$SDK_DIR/components/bk_wifi/include/bk_private>",
        "+<$SDK_DIR/components/bk_netif/include>",
        "+<$SDK_DIR/components/bk_cli/include>",
        "+<$SDK_DIR/components/bk_cli/include/bk_private>",
        "+<$SDK_DIR/components/bk_cli>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/hostapd>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/src/utils>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/src/ap>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/src/common>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/src/drivers>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/src>",
        "+<$SDK_DIR/components/wpa_supplicant-2.10/wpa_supplicant>",
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
        "+<$SDK_DIR/middleware/driver/saradc/v1_1>",
        "+<$SDK_DIR/middleware/driver/aon_wdt>",
        "+<$SDK_DIR/middleware/driver/pwm/v1px>",
        "+<$SDK_DIR/middleware/driver/pmu>",
        "+<$SDK_DIR/middleware/driver/mailbox>",
        "+<$SDK_DIR/middleware/driver/spinlock>",
        "+<$SDK_DIR/middleware/driver/rtc>",
        "+<$SDK_DIR/middleware/compal/common_io/include>",
        "+<$SDK_DIR/middleware/driver/wdt>",
        "+<$SDK_DIR/middleware/driver/timer>",
        "+<$SDK_DIR/middleware/driver/spi>",
        "+<$SDK_DIR/middleware/driver/bk7236>",
        "+<$SDK_DIR/middleware/driver/ckmn>",
        "+<$SDK_DIR/middleware/arch/cm33/include>",
        "+<$SDK_DIR/middleware/arch/cm33>",
        "+<$SDK_DIR/components/bk_system/soc/bk7231n>",
        "+<$SDK_DIR/components/bk_rtos/include>",
        "+<$SDK_DIR/components/bk_rtos/freertos>",
        "+<$SDK_DIR/components/cmsis/CMSIS_5/CMSIS/Core/Include>",
        "+<$SDK_DIR/components/cmsis/CMSIS_5/Device/Beken/bk7236xx/Include>",
        "+<$SDK_DIR/components/cmsis/CMSIS_5/Device/Beken/armstar>",
        "+<$SDK_DIR/components/bk_ps/include>",
        "+<$SDK_DIR/components/coredump/cm_backtrace>",
        "+<$SDK_DIR/components/coredump/cm_backtrace/Languages/en-US>",
        "+<$SDK_DIR/components/bk_phy/include>",
        "+<$SDK_DIR/components/bk_cli/uart_debug>",
        "+<$SDK_DIR/middleware/boards/bk7236/vnd_cal>",
        "+<$SDK_DIR/middleware/boards/bk7236/vnd_cal/include>",
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
        "+<$SDK_DIR/components/mbedtls/mbedtls-port/inc>",
        "+<$SDK_DIR/components/mbedtls/mbedtls/include>",
        "+<$SDK_DIR/components/mbedtls/mbedtls_ui>",
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
        "+<bk7236/hal/adc_hal.c>",
        "+<common/hal/aon_rtc_hal_64bit.c>",
        "+<common/hal/ckmn_hal.c>",
        "+<common/hal/dma_hal.c>",
        "+<common/hal/efuse_hal.c>",
        "+<common/hal/flash_hal.c>",
        "+<common/hal/gpio_hal.c>",
        "+<common/hal/i2c_hal.c>",
        "+<common/hal/icu_hal.c>",
        "+<common/hal/aon_wdt_hal.c>",
        "+<common/hal/pwm_hal_v1px.c>",
        "+<common/hal/sdio_hal.c>",
        "+<common/hal/sdmadc_hal.c>",
        "+<bk7236/hal/spi_hal.c>",
        "+<common/hal/timer_hal.c>",
        "+<common/hal/trng_hal.c>",
        "+<common/hal/uart_hal.c>",
        "+<bk7236/hal/wdt_hal.c>",
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
        "+<CMSIS_5/Device/Beken/bk7236xx/Source/startup_bk7236xx.c>",
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
        "+<bk7236/gpio_driver.c>",
        "+<bk7236/interrupt.c>",
        "+<bk7236/interrupt_statis.c>",
        "+<chip_support/chip_support.c>",
        "+<ckmn/ckmn_driver.c>",
        "+<common/driver.c>",
        "+<dsp/dsp.c>",
        "+<efuse/efuse_driver.c>",
        "+<flash/flash_bypass.c>",
        "+<flash/flash_driver.c>",
        "+<flash/flash_driver_ext.c>",
        "+<flash/flash_notify.c>",
        "+<flash/flash_partition.c>",
        "+<general_dma/v2px/dma_driver.c>",
        "+<gpio/gpio_driver_base.c>",
        "+<hw_rotate/rott_driver.c>",
        "+<icu/icu_driver.c>",
        "+<icu/interrupt_base.c>",
        "+<pmu/aon_pmu_driver.c>",
        "+<pwm/v1px/pwm_driver.c>",
        "+<pwr_clk/pwr_clk.c>",
        "+<pwr_clk/rosc_32k.c>",
        "+<pwr_clk/rosc_ppm.c>",
        "+<reset_reason/reset_reason.c>",
        "+<rtc/aon_rtc_driver_64bit.c>",
        "+<saradc/v1_1/adc_driver.c>",
        "+<saradc/v1_1/adc_statis.c>",
        "+<aon_wdt/aon_wdt_driver.c>",
        "+<flash/CheckSumUtils.c>",
        "+<gpio/gpio.c>",
        "+<spi/spi_rz.c>",
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
    options=dict(
        # bk_patch/signal.h replaces <signal.h>, only for wpa_supplicant and bk_wifi
        CCFLAGS=["-I$SDK_DIR/components/wpa_supplicant-2.10/bk_patch"],
    ),
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
        "+<bk_platform.c>",
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

# Sources - components/ota
queue.AddLibrary(
    name="armino_ota",
    base_dir=join("$SDK_DIR", "components/ota"),
    srcs=[
        "+<CheckSumUtils.c>",
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
        "+<src/bk_pm.c>",
        "+<src/core/bk_pm_core.c>",
        "+<src/services/bk_pm_clock.c>",
        "+<src/services/bk_pm_debug.c>",
        "+<src/services/bk_pm_misc.c>",
        "+<src/services/bk_pm_power.c>",
        "+<src/services/bk_pm_sleep.c>",
        "+<src/services/bk_pm_sleep_callback.c>",
        "+<src/services/bk_pm_thread.c>",
        "+<src/services/bk_pm_wakeup_source.c>",
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

# Sources - components/wpa_supplicant-2.10
queue.AddLibrary(
    name="armino_wpa_supplicant_2_10",
    base_dir=join("$SDK_DIR", "components/wpa_supplicant-2.10"),
    options=dict(
        # own headers first, like in ARMINO (list.h, ip_addr.h, cli.h, config.h collide)
        CCFLAGS=[
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/hostapd",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/src/utils",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/src/ap",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/src/common",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/src/drivers",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/src",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/wpa_supplicant",
            "-iquote$SDK_DIR/components/wpa_supplicant-2.10/bk_patch",
            # bk_patch/signal.h replaces <signal.h>, only for wpa_supplicant and bk_wifi
            "-I$SDK_DIR/components/wpa_supplicant-2.10/bk_patch",
            # bk_idk v2.2.1 crypto_mbedtls.c only defines this for mbedTLS 3.x (PSA)
            "'-DMBEDTLS_PRIVATE(member)=member'",
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
        "+<src/crypto/aes-siv.c>",
        "+<src/crypto/aes-unwrap.c>",
        "+<src/crypto/aes-wrap.c>",
        "+<src/crypto/crypto_mbedtls.c>",
        "+<src/crypto/dh_group5.c>",
        "+<src/crypto/dh_groups.c>",
        "+<src/crypto/sha1-prf.c>",
        "+<src/crypto/sha256-kdf.c>",
        "+<src/crypto/sha256-prf.c>",
        "+<src/crypto/sha384-kdf.c>",
        "+<src/crypto/sha384-prf.c>",
        "+<src/crypto/sha512-kdf.c>",
        "+<src/crypto/sha512-prf.c>",
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

# Sources - components/bk_netif
queue.AddLibrary(
    name="armino_bk_netif",
    base_dir=join("$SDK_DIR", "components/bk_netif"),
    srcs=[
        "+<bk_netif.c>",
    ],
)

# Sources - components/mbedtls
queue.AddLibrary(
    name="armino_mbedtls",
    base_dir=join("$SDK_DIR", "components/mbedtls"),
    # mbedtls-port/src/tls_net.c is left out: port/net_sockets.c provides the same
    # mbedtls_net_*() functions (and mbedtls_net_poll())
    srcs=[
        "+<mbedtls-port/src/timing_alt.c>",
        "+<mbedtls-port/src/tls_certificate.c>",
        "+<mbedtls-port/src/tls_client.c>",
        "+<mbedtls-port/src/tls_hardware.c>",
        "+<mbedtls-port/src/tls_mem.c>",
        "+<mbedtls/library/aes.c>",
        "+<mbedtls/library/aesni.c>",
        "+<mbedtls/library/arc4.c>",
        "+<mbedtls/library/aria.c>",
        "+<mbedtls/library/asn1parse.c>",
        "+<mbedtls/library/asn1write.c>",
        "+<mbedtls/library/base64.c>",
        "+<mbedtls/library/bignum.c>",
        "+<mbedtls/library/blowfish.c>",
        "+<mbedtls/library/camellia.c>",
        "+<mbedtls/library/ccm.c>",
        "+<mbedtls/library/certs.c>",
        "+<mbedtls/library/chacha20.c>",
        "+<mbedtls/library/chachapoly.c>",
        "+<mbedtls/library/cipher.c>",
        "+<mbedtls/library/cipher_wrap.c>",
        "+<mbedtls/library/cmac.c>",
        "+<mbedtls/library/constant_time.c>",
        "+<mbedtls/library/ctr_drbg.c>",
        "+<mbedtls/library/debug.c>",
        "+<mbedtls/library/des.c>",
        "+<mbedtls/library/dhm.c>",
        "+<mbedtls/library/ecdh.c>",
        "+<mbedtls/library/ecdsa.c>",
        "+<mbedtls/library/ecjpake.c>",
        "+<mbedtls/library/ecp.c>",
        "+<mbedtls/library/ecp_curves.c>",
        "+<mbedtls/library/entropy.c>",
        "+<mbedtls/library/entropy_poll.c>",
        "+<mbedtls/library/error.c>",
        "+<mbedtls/library/gcm.c>",
        "+<mbedtls/library/havege.c>",
        "+<mbedtls/library/hkdf.c>",
        "+<mbedtls/library/hmac_drbg.c>",
        "+<mbedtls/library/md.c>",
        "+<mbedtls/library/md2.c>",
        "+<mbedtls/library/md4.c>",
        "+<mbedtls/library/md5.c>",
        "+<mbedtls/library/memory_buffer_alloc.c>",
        "+<mbedtls/library/net_sockets.c>",
        "+<mbedtls/library/nist_kw.c>",
        "+<mbedtls/library/oid.c>",
        "+<mbedtls/library/padlock.c>",
        "+<mbedtls/library/pem.c>",
        "+<mbedtls/library/pk.c>",
        "+<mbedtls/library/pk_wrap.c>",
        "+<mbedtls/library/pkcs11.c>",
        "+<mbedtls/library/pkcs12.c>",
        "+<mbedtls/library/pkcs5.c>",
        "+<mbedtls/library/pkparse.c>",
        "+<mbedtls/library/pkwrite.c>",
        "+<mbedtls/library/platform.c>",
        "+<mbedtls/library/platform_util.c>",
        "+<mbedtls/library/poly1305.c>",
        "+<mbedtls/library/psa_crypto.c>",
        "+<mbedtls/library/psa_crypto_se.c>",
        "+<mbedtls/library/psa_crypto_slot_management.c>",
        "+<mbedtls/library/psa_crypto_storage.c>",
        "+<mbedtls/library/psa_its_file.c>",
        "+<mbedtls/library/ripemd160.c>",
        "+<mbedtls/library/rsa.c>",
        "+<mbedtls/library/rsa_internal.c>",
        "+<mbedtls/library/sha1.c>",
        "+<mbedtls/library/sha256.c>",
        "+<mbedtls/library/sha512.c>",
        "+<mbedtls/library/ssl_cache.c>",
        "+<mbedtls/library/ssl_ciphersuites.c>",
        "+<mbedtls/library/ssl_cli.c>",
        "+<mbedtls/library/ssl_cookie.c>",
        "+<mbedtls/library/ssl_msg.c>",
        "+<mbedtls/library/ssl_srv.c>",
        "+<mbedtls/library/ssl_ticket.c>",
        "+<mbedtls/library/ssl_tls.c>",
        "+<mbedtls/library/threading.c>",
        "+<mbedtls/library/timing.c>",
        "+<mbedtls/library/version.c>",
        "+<mbedtls/library/version_features.c>",
        "+<mbedtls/library/x509.c>",
        "+<mbedtls/library/x509_create.c>",
        "+<mbedtls/library/x509_crl.c>",
        "+<mbedtls/library/x509_crt.c>",
        "+<mbedtls/library/x509_csr.c>",
        "+<mbedtls/library/x509write_crt.c>",
        "+<mbedtls/library/x509write_csr.c>",
        "+<mbedtls/library/xtea.c>",
        "+<mbedtls_ui/sl_tls.c>",
        "+<port/net_sockets.c>",
    ],
)

# Sources - lwIP
queue.AddExternalLibrary("lwip", port="armino")

# Libs & linker config
queue.AppendPublic(
    LIBPATH=[
        join("$SDK_DIR", "components", "bk_libs", SOC, "libs"),
    ],
    LIBS=[
        "bk_phy",
        "bk_phy_info",
        "ble_wifi_exchange",
        "wifi",
        "bk_coex",
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

# Rename Arduino's delay() to delayMilliseconds() - ARMINO has its own delay()
env.Append(
    CPPDEFINES=[
        ("'delay(ms)'", "'delayMilliseconds(ms)'"),
    ],
)

# Check used OTA format
ota_format = board.get("build.bkota.format")

# Main firmware outputs and actions
image_app_crc = "${BUILD_DIR}/image_${MCULC}_app.${FLASH_APP_OFFSET}.crc"
image_ota_ug = "${BUILD_DIR}/image_${MCULC}_app.ota.ug.bin"
image_diff2ya_info = "${BUILD_DIR}/image_${MCULC}_app.diff2ya.dat"
env.Replace(
    # linker command (CRC + packaging)
    LINK='${LTCHIPTOOL} link2bin ${BOARD_JSON} "" ""',
    # UF2OTA input list
    UF2OTA=[
        # app binary image (crc) for flasher
        f"{image_app_crc}=flasher:app",
        # diff2ya format - use UG image for 'download' and flash info partition
        ota_format == "diff2ya" and f"{image_ota_ug}=device:download",
        ota_format == "diff2ya" and f"{image_diff2ya_info}=device:diff2ya",
    ],
)
