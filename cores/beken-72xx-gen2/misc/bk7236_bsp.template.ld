OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
__MSP_STACK_SIZE = (4 << 10);
__MIN_HEAP_SIZE = (90 << 10);
__MPU_PROTECT_SIZE = 0x0;
__SWAP_SIZE = (0);
__CPU0_APP_RAM_BASE = (0x28000000 + (0));
__CPU0_APP_RAM_SIZE = 0xA0000;
__CPU0_APP_IRAM_OFFSET = 0x20000000;
__CPU0_APP_IRAM_BASE = __CPU0_APP_RAM_BASE - __CPU0_APP_IRAM_OFFSET;
__CPU0_APP_IRAM_SIZE = __CPU0_APP_RAM_SIZE;
__CPU0_APP_VIRTUAL_CODE_START = 0x10000;
__CPU0_APP_VIRTUAL_CODE_SIZE = 0x21e1e0;
MEMORY
{
  FLASH (rx) : ORIGIN = ((0x02000000 + (0)) + __CPU0_APP_VIRTUAL_CODE_START), LENGTH = __CPU0_APP_VIRTUAL_CODE_SIZE
  IRAM (rx) : ORIGIN = __CPU0_APP_IRAM_BASE, LENGTH = __CPU0_APP_IRAM_SIZE
  RAM (rwx) : ORIGIN = __CPU0_APP_RAM_BASE, LENGTH = __CPU0_APP_RAM_SIZE - __SWAP_SIZE
  SWAP (rwx) : ORIGIN = __CPU0_APP_RAM_BASE + __CPU0_APP_RAM_SIZE - __SWAP_SIZE, LENGTH = __SWAP_SIZE
  ITCM (rwx) : ORIGIN = (0x00000000 + (0)) + __MPU_PROTECT_SIZE, LENGTH = 0x4000 - __MPU_PROTECT_SIZE
  DTCM (rwx) : ORIGIN = (0x20000000 + (0)) + 0, LENGTH = 0x3FE0
  PSRAM (rwx) : ORIGIN = (0x60000000 + (0)), LENGTH = 0x4000000
}
ENTRY(Reset_Handler)
SECTIONS
{
    ASSERT((. == ALIGN(512)), "vector table address align fault.")
    .vectors :
    {
        __vector_table = .;
        KEEP(*(.vectors))
        *(.fix.reset_entry)
    } > FLASH
    .gnu.sgstubs ALIGN(32) : ALIGN(32)
    {
        *(.gnu.sgstubs*)
        . = ALIGN(32);
    } > FLASH
    .text :
    {
        . = ALIGN(4);
        _stext = .;
 EXCLUDE_FILE
  (
  *tasks.c.o *queue.c.o *list.c.o *timers.c.o *port.c.o *rtos_pub.c.o *heap_4.c.o *mem_arch.c.o *gpio_driver_base.c.o *flash_driver.c.o *ckmn_driver.c.o *arch_interrupt.c.o *hal_machw.c.o *interrupts.c.o
  *rw_task.c.o *me_strategy.c.o *chan.c.o *vif_mgmt.c.o *rxl_cntrl.c.o *rxl_hwdesc.c.o *mm_timer.c.o *ke_event.c.o *rwnx.c.o *rwnx_rx.c.o *ps.c.o *mm.c.o *rf_cntrl.c.o *mcc.c.o
  *phy_karst_bk7236.c.o *crm.c.o *bk7236_cal.c.o *manual_cal_bk7236.c.o *bk_cal_common.c.o *nv_parameters.c.o *macif_fhost.c.o *rc_driver.c.o *sys_wifi_driver.c.o *wifi_v2.c.o *bk_feature.c.o
  *pm.c.o *sys_ps_driver.c.o *sys_pm_hal.c.o *bk_wifi_adapter.c.o *net_param.c.o *wifi_api_wrapper.c.o *bk_rf_adapter.c.o *bk_phy_adapter.c.o
  *shell_task.c.o *shell_uart.c.o *uart_driver.c.o *uart_hal.c.o
  )
 *(.text.* *.rodata *.rodata.*)
        . = ALIGN(4);
        __devconfig_start = .;
        *(".devconfig.*")
        KEEP(*(SORT_BY_NAME(".devconfig*")))
        __devconfig_end = .;
        . = ALIGN(4);
        __apps_start = .;
        KEEP (*(.apps_data))
        __apps_end = .;
        _etext = .;
        . = ALIGN(4);
    } > FLASH
    .a_device_null :
    {
      KEEP(*(.a_deviceobj_null))
    } > FLASH
    .a_devices :
    {
      __device_start = .;
      KEEP(*(.a_deviceobj_*))
      __device_end = .;
    } > FLASH
    .a_init_entries :
    {
      __a_init_start = .;
      KEEP(*(.a_init_entry_*))
      __a_init_end = .;
    } > FLASH
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        . = ALIGN(4);
    } > FLASH
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;
    .init_array :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } > FLASH
    .copy.table :
    {
        . = ALIGN(4);
        __copy_table_start__ = .;
        LONG (__iram_flash_begin)
        LONG (__iram_start__)
        LONG ((__iram_end__ - __iram_start__) / 4)
        LONG (__data_flash_begin)
        LONG (__data_start__)
        LONG ((__data_end__ - __data_start__) / 4)
        LONG (__video_cache_text)
        LONG (__video_cache_data_start__)
        LONG ((__video_cache_data_end__ - __video_cache_data_start__) / 4)
        LONG (__itcm_text)
        LONG (__itcm_start__)
        LONG ((__itcm_end__ - __itcm_start__) / 4)
        LONG (__dtcm_content)
        LONG (__dtcm_start__)
        LONG ((__dtcm_end__ - __dtcm_start__) / 4)
        __copy_table_end__ = .;
    } > FLASH
    .zero.table :
    {
        . = ALIGN(4);
        __zero_table_start__ = .;
        LONG (_bss_start)
        LONG ((_bss_end - _bss_start) / 4)
        LONG (_heap_start)
        LONG ((_heap_end - _heap_start) / 4)
        LONG (__video_cache_bss_start__)
        LONG ((__video_cache_bss_end__ - __video_cache_bss_start__) / 4)
        __zero_table_end__ = .;
    } > FLASH
    .itcm :
    {
        . = ALIGN(4);
        PROVIDE(__itcm_text = LOADADDR(.itcm));
        __itcm_start__ = .;
        KEEP(*(.null_trap_handler))
        *(.itcm_section*)
        *(.itcm_sec_code*)
        . = ALIGN(4);
        __itcm_end__ = .;
    } > ITCM AT > FLASH
    .dtcm :
    {
        . = ALIGN(4);
        PROVIDE(__dtcm_content = LOADADDR(.dtcm));
        __dtcm_start__ = .;
        *(.dtcm)
        *(.dtcm_section*)
        *(.dtcm_sec_data*)
        . = ALIGN(4);
        __dtcm_end__ = .;
    } > DTCM AT > FLASH
    .iram :
    {
        . = ALIGN(512);
        PROVIDE(__iram_flash_begin = LOADADDR(.iram));
        __iram_start__ = .;
        . = ALIGN(512);
        __vector_iram_table = .;
        KEEP(*(.vectors_iram))
        . = ALIGN(512);
        *(.itcm)
        *(.iram)
        *(.interrupt)
        *tasks.c.o(.text.* *.rodata *.rodata.*)
        *queue.c.o(.text.* *.rodata *.rodata.*)
        *list.c.o(.text.* *.rodata *.rodata.*)
        *timers.c.o(.text.* *.rodata *.rodata.*)
        *port.c.o(.text.* *.rodata *.rodata.*)
        *rtos_pub.c.o(.text.* *.rodata *.rodata.*)
        *heap_4.c.o(.text.* *.rodata *.rodata.*)
        *mem_arch.c.o(.text.* *.rodata *.rodata.*)
        *gpio_driver_base.c.o(.text.* *.rodata *.rodata.*)
        *flash_driver.c.o(.text.* *.rodata *.rodata.*)
        *ckmn_driver.c.o(.text.* *.rodata *.rodata.*)
        *arch_interrupt.c.o(.text.* *.rodata *.rodata.*)
        *hal_machw.c.o(.text.* *.rodata *.rodata.*)
        *interrupts.c.o(.text.* *.rodata *.rodata.*)
        *rw_task.c.o(.text.* *.rodata *.rodata.*)
        *ke_event.c.o(.text.* *.rodata *.rodata.*)
        *me_strategy.c.o(.text.* *.rodata *.rodata.*)
        *chan.c.o(.text.* *.rodata *.rodata.*)
        *vif_mgmt.c.o(.text.* *.rodata *.rodata.*)
        *rxl_cntrl.c.o(.text.* *.rodata *.rodata.*)
        *rxl_hwdesc.c.o(.text.* *.rodata *.rodata.*)
        *mm_time.c.o(.text.* *.rodata *.rodata.*)
        *rwnx.c.o(.text.* *.rodata *.rodata.*)
        *rwnx_rx.c.o(.text.* *.rodata *.rodata.*)
        *ps.c.o(.text.* *.rodata *.rodata.*)
        *mm.c.o(.text.* *.rodata *.rodata.*)
        *rf_cntrl.c.o(.text.* *.rodata *.rodata.*)
        *mcc.c.o(.text.* *.rodata *.rodata.*)
        *phy_karst_bk7236.c.o(.text.* *.rodata *.rodata.*)
        *crm.c.o (.text.* *.rodata *.rodata.*)
        *bk7236_cal.c.o(.text.* *.rodata *.rodata.*)
        *manual_cal_bk7236.c.o(.text.* *.rodata *.rodata.*)
        *bk_cal_common.c.o(.text.* *.rodata *.rodata.*)
        *nv_parameters.c.o(.text.* *.rodata *.rodata.*)
        *macif_fhost.c.o(.text.* *.rodata *.rodata.*)
        *rc_driver.c.o(.text.* *.rodata *.rodata.*)
        *sys_wifi_driver.c.o(.text.* *.rodata *.rodata.*)
        *wifi_v2.c.o(.text.* *.rodata *.rodata.*)
        *bk_feature.c.o(.text.* *.rodata *.rodata.*)
        *pm.c.o(.text.* *.rodata *.rodata.*)
        *sys_ps_driver.c.o(.text.* *.rodata *.rodata.*)
        *sys_pm_hal.c.o(.text.* *.rodata *.rodata.*)
        *bk_wifi_adapter.c.o(.text.* *.rodata *.rodata.*)
        *net_param.c.o(.text.* *.rodata *.rodata.*)
        *wifi_api_wrapper.c.o(.text.* *.rodata *.rodata.*)
        *bk_rf_adapter.c.o(.text.* *.rodata *.rodata.*)
        *bk_phy_adapter.c.o(.text.* *.rodata *.rodata.*)
        *shell_task.c.o(.text.* *.rodata *.rodata.*)
        *shell_uart.c.o(.text.* *.rodata *.rodata.*)
        *uart_driver.c.o(.text.* *.rodata *.rodata.*)
        *uart_hal.c.o(.text.* *.rodata *.rodata.*)
        . = ALIGN(4);
        __iram_end__ = .;
    } > IRAM AT > FLASH
    .ramcode :
    {
        . = . + SIZEOF(.iram);
        . = ALIGN(512);
    } > RAM AT > FLASH
    .data :
    {
        PROVIDE(__etext = LOADADDR(.data));
        PROVIDE(__data_flash_begin = LOADADDR(.data));
        . = ALIGN(4);
        __data_start__ = .;
        *(.data)
        *(".data.*")
        *(.sdata)
        *(.video_spec_data*)
        *(.gnu.linkonce.d*)
        . = ALIGN(4);
        __data_end__ = .;
    } > RAM AT > FLASH
    _data_flash_begin = __data_flash_begin;
    _data_ram_begin = __data_start__;
    _data_ram_end = .;
    s_rom_end = LOADADDR(.data) + SIZEOF(.data);
    .uninitialized (NOLOAD):
    {
        . = ALIGN(32);
        __uninitialized_start = .;
        *(.uninitialized)
        *(".uninitialized.*")
        KEEP(*(.keep.uninitialized))
        . = ALIGN(32);
        __uninitialized_end = .;
    } > RAM
    .bss (NOLOAD):
    {
        . = ALIGN(4);
        _bss_start = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > RAM
    bss_size = _bss_end - _bss_start;
 _msp_total_size = __MSP_STACK_SIZE;
    .heap (COPY) :
    {
        . = ALIGN(8);
        _heap_start = .;
        . = . + (ORIGIN(RAM) + LENGTH(RAM) - _msp_total_size - _heap_start - 8);
        . = ALIGN(8);
        _heap_end = .;
    } > RAM
    .stack (ORIGIN(RAM) + LENGTH(RAM) - __MSP_STACK_SIZE) (COPY) :
    {
        . = ALIGN(8);
        _sstack = .;
        __StackLimit = .;
        . = . + __MSP_STACK_SIZE;
        . = ALIGN(8);
        __StackTop = .;
        _estack = .;
    } > RAM
    .swap ORIGIN(SWAP):
    {
        . = ALIGN(8);
        _swap_start = .;
        * (.swap_data)
        * (.swap_data*)
        . = ALIGN(4);
        _swap_end = .;
    } > SWAP AT > FLASH
    .video.cache.data :
    {
        . = ALIGN(4);
        PROVIDE(__video_cache_text = LOADADDR(.video.cache.data));
        __video_cache_data_start__ = .;
        *(.video_cache_data)
        *(.video_cache_data*)
        . = ALIGN(4);
        __video_cache_data_end__ = .;
    } > PSRAM AT > FLASH
    .video_cache_bss (NOLOAD):
    {
        . = ALIGN(4);
        __video_cache_bss_start__ = .;
        *(.video_cache_bss)
        *(.video_cache_bss*)
        . = ALIGN(4);
        __video_cache_bss_end__ = .;
    } > PSRAM
    __MSPTop = ORIGIN(RAM) + LENGTH(RAM);
    __MSPLimit = __MSPTop - __MSP_STACK_SIZE;
    ASSERT((s_rom_end < ORIGIN(FLASH) + LENGTH(FLASH)), "ROM overflow!!!")
    ASSERT(((ORIGIN(RAM) + LENGTH(RAM)) > (_heap_start + __MSP_STACK_SIZE + 8)), "Stack overflowed with bss")
    ASSERT(((_heap_end - _heap_start) >= __MIN_HEAP_SIZE), "Heap smaller than minimize size 90K!!!")
}
