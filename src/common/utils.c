/**
 * 工具函数
 */

#include "../include/uart_to_x.h"
#include <ctype.h>

/**
 * CRC16-CCITT 计算
 */
uint16_t crc16(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/**
 * 十六进制字符转数值
 */
static int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/**
 * 十六进制字符串转字节数组
 * 输入: "48656C6C6F" 或 "48 65 6C 6C 6F"
 * 返回: 转换的字节数
 */
int hex_to_bytes(const char *hex, uint8_t *bytes, size_t max_len) {
    size_t count = 0;
    const char *p = hex;

    while (*p && count < max_len) {
        // 跳过空格
        while (*p && isspace(*p)) p++;
        if (!*p) break;

        // 跳过 "0x" 前缀
        if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
            p += 2;
        }

        int high = hex_char_to_int(*p);
        if (high < 0) break;
        p++;

        int low = hex_char_to_int(*p);
        if (low < 0) {
            // 单个字符，视为低4位
            bytes[count++] = high;
        } else {
            bytes[count++] = (high << 4) | low;
            p++;
        }
    }

    return count;
}

/**
 * 字节数组转十六进制字符串
 */
void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex) {
    static const char hex_chars[] = "0123456789ABCDEF";

    for (size_t i = 0; i < len; i++) {
        hex[i * 3] = hex_chars[(bytes[i] >> 4) & 0x0F];
        hex[i * 3 + 1] = hex_chars[bytes[i] & 0x0F];
        hex[i * 3 + 2] = ' ';
    }
    if (len > 0) {
        hex[len * 3 - 1] = '\0';
    } else {
        hex[0] = '\0';
    }
}

/**
 * 简单的字符串比较 (不区分大小写)
 */
int strcasecmp_simple(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        char c1 = tolower(*s1);
        char c2 = tolower(*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return *s1 - *s2;
}
