#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "tusb.h"

#define REPORT_ID_TOUCH  0x01

// 触摸屏输入报告结构（必须与 HID 报告描述符完全一致）
typedef struct __attribute__((packed))
{
    uint8_t  report_id;      // = REPORT_ID_TOUCH
    uint8_t  tip : 1;        // 触摸标志
    uint8_t  in_range : 1;   // 触摸有效范围
    uint8_t  confidence : 1; // 数据置信度
    uint8_t  reserved1 : 5;  // 填充位
    uint8_t  contact_id;     // 触点标识
    uint16_t x;              // X 坐标 (0~32767)
    uint16_t y;              // Y 坐标 (0~32767)
    uint16_t scan_time;      // 扫描时间（Windows 推荐字段）
} touch_report_t;

uint8_t const * tud_descriptor_device_cb(void);
uint8_t const * tud_descriptor_configuration_cb(uint8_t index);
uint16_t const * tud_descriptor_string_cb(uint8_t index, uint16_t langid);
uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf);

#endif
