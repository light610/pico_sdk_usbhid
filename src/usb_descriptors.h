#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "tusb.h"

#define REPORT_ID_TOUCH  0x01
#define REPORT_ID_FEATURE 0x02   // Feature Report ID

// 触摸屏输入报告（必须与描述符完全一致）
typedef struct __attribute__((packed))
{
    uint8_t  report_id;      // = REPORT_ID_TOUCH
    uint8_t  tip : 1;        // 触摸标志
    uint8_t  in_range : 1;   // 触摸有效范围
    uint8_t  confidence : 1; // 数据置信度
    uint8_t  reserved1 : 5;  // 填充至 1 字节
    uint8_t  contact_id;     // 触点标识
    uint16_t x;              // X 坐标 (0~32767)
    uint16_t y;              // Y 坐标 (0~32767)
    uint8_t  scan_time;      // 扫描时间（可选，但 Windows 推荐）
} touch_report_t;

// 特征报告（最大触点数量）
typedef struct __attribute__((packed))
{
    uint8_t report_id;       // = REPORT_ID_FEATURE
    uint8_t contact_count_max;
} feature_report_t;

uint8_t const * tud_descriptor_device_cb(void);
uint8_t const * tud_descriptor_configuration_cb(uint8_t index);
uint16_t const * tud_descriptor_string_cb(uint8_t index, uint16_t langid);
uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf);

#endif
