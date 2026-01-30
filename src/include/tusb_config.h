/*
 * TinyUSB 配置文件
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// 通用配置
//--------------------------------------------------------------------

#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU                OPT_MCU_RP2040
#endif
#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE
#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                 OPT_OS_NONE
#endif

// 调试级别
#define CFG_TUSB_DEBUG              0

// DMA支持
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN          __attribute__((aligned(4)))

//--------------------------------------------------------------------
// 设备配置
//--------------------------------------------------------------------

#define CFG_TUD_ENABLED             1
#define CFG_TUD_MAX_SPEED           OPT_MODE_FULL_SPEED

// 端点0大小
#define CFG_TUD_ENDPOINT0_SIZE      64

//--------------------------------------------------------------------
// 设备类配置
//--------------------------------------------------------------------

// CDC 类
#define CFG_TUD_CDC                 1
#define CFG_TUD_CDC_RX_BUFSIZE      512
#define CFG_TUD_CDC_TX_BUFSIZE      512

// 禁用其他类
#define CFG_TUD_MSC                 0
#define CFG_TUD_HID                 0
#define CFG_TUD_MIDI                0
#define CFG_TUD_VENDOR              0

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
