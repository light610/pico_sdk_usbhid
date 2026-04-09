#include "usb_descriptors.h"
#include "pico/unique_id.h"
#include "tusb.h"
#include <string.h>
#include <stdio.h>

// 使用更常见的 VID/PID（例如微软测试 VID）
#define USB_VID       0x045E
#define USB_PID       0x0791
#define USB_BCD       0x0200

static const tusb_desc_device_t device_desc = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &device_desc;
}

// 配置描述符
enum { ITF_NUM_HID, ITF_NUM_TOTAL };
#define EPNUM_HID   0x81
#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

// 符合 Windows 精确触摸屏规范的报告描述符
static const uint8_t hid_report_desc[] = {
    // 主 Collection
    0x05, 0x0D,        // Usage Page (Digitizers)
    0x09, 0x04,        // Usage (Touch Screen)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_TOUCH, //   Report ID (1)

    // 手指集合
    0x09, 0x22,        //   Usage (Finger)
    0xA1, 0x02,        //   Collection (Logical)

    // Tip Switch
    0x09, 0x42,        //     Usage (Tip Switch)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    // In Range
    0x09, 0x32,        //     Usage (In Range)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    // Confidence
    0x09, 0x47,        //     Usage (Confidence)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    // 填充 5 bits
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x05,        //     Report Count (5)
    0x81, 0x03,        //     Input (Const, Var, Abs)

    // Contact ID
    0x09, 0x51,        //     Usage (Contact ID)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x0F,        //     Logical Maximum (15)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    // X 坐标
    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    // Y 坐标
    0x09, 0x31,        //     Usage (Y)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    // Scan Time (可选但推荐)
    0x05, 0x0D,        //     Usage Page (Digitizers)
    0x09, 0x56,        //     Usage (Scan Time)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    0xC0,              //   End Collection (Logical)

    // 特征报告（最大触点数量）
    0x09, 0x55,        //   Usage (Contact Count Maximum)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0xB1, 0x02,        //   Feature (Data, Var, Abs)

    0xC0               // End Collection
};

static const uint8_t config_desc[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(hid_report_desc), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 10)
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    return config_desc;
}

// 字符串描述符
static char serial_str[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
static const char *string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },
    "Microsoft",                // 与 VID 045E 匹配
    "Pico Touch Screen",
    serial_str,
};

uint16_t const * tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint16_t desc_str[32];
    uint8_t len = 0;
    if (index == 0) {
        memcpy(&desc_str[1], string_desc_arr[0], 2);
        len = 1;
    } else {
        if (index >= sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) return NULL;
        const char *str = string_desc_arr[index];
        if (index == 3) {
            pico_unique_board_id_t id;
            pico_get_unique_board_id(&id);
            for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i++)
                sprintf(&serial_str[i*2], "%02X", id.id[i]);
            str = serial_str;
        }
        len = strlen(str);
        if (len > 31) len = 31;
        for (uint8_t i = 0; i < len; i++) desc_str[1+i] = str[i];
    }
    desc_str[0] = (TUSB_DESC_STRING << 8) | (2*len + 2);
    return desc_str;
}

uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf) {
    (void) itf;
    return hid_report_desc;
}

// 处理 Feature Report 请求（必须正确响应）
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void) itf;
    if (report_type == HID_REPORT_TYPE_FEATURE && report_id == 0x02) {
        // 返回 Contact Count Maximum = 1
        buffer[0] = 0x02; // report_id
        buffer[1] = 0x01; // max contacts
        return 2;
    }
    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type,
                           uint8_t const* buffer, uint16_t bufsize) {
    (void) itf; (void) report_id; (void) report_type; (void) buffer; (void) bufsize;
}
