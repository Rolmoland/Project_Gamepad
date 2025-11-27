/**
 * @file gamepad_app.c
 * @brief 游戏手柄应用层实现
 * @details 定期扫描按键,映射到USB HID手柄报告并发送
 */

#include "gamepad_app.h"
#include "key_app.h"
#include "usb_app.h"
#include <rtthread.h>

/* ================ 全局变量 ================ */

static rt_thread_t gamepad_thread = RT_NULL;
static uint16_t current_buttons = 0;

/* ================ 内部函数 ================ */

/**
 * @brief 游戏手柄任务线程
 */
static void gamepad_thread_entry(void *parameter)
{
    usb_gamepad_report_t *report;
    uint8_t key_index;
    uint8_t last_key = 0xFF;

    rt_kprintf("[GAMEPAD] Thread started\n");

    while (1)
    {
        /* 读取当前按键状态 */
        key_index = key_read();

        /* 只在按键状态变化时发送报告 */
        if (key_index != last_key)
        {
            /* 获取USB报告缓冲区 */
            report = hid_gamepad_get_report();

            /* 核心逻辑: 按键索引 -> 按钮位映射 */
            if (key_index != 0xFF)
            {
                /* 有按键按下: 设置对应位 */
                current_buttons = (1 << key_index);
            }
            else
            {
                /* 无按键: 清零所有按钮 */
                current_buttons = 0;
            }

            /* 更新报告数据 */
            report->buttons = current_buttons;

            /* 保持其他数据为中立状态(摇杆居中,扳机释放) */
            report->left_x = 0;
            report->left_y = 0;
            report->right_x = 0;
            report->right_y = 0;
            report->left_trigger = 0;
            report->right_trigger = 0;
            report->hat = GAMEPAD_HAT_CENTER;

            /* 发送USB报告 */
            if (hid_gamepad_is_configured(GAMEPAD_USB_BUS_ID))
            {
                int ret = hid_gamepad_send_report(GAMEPAD_USB_BUS_ID, NULL);

                if (ret == 0 && key_index != 0xFF)
                {
                    rt_kprintf("[GAMEPAD] Button %d pressed (bit %d)\n", key_index, key_index);
                }
                else if (ret == 0 && key_index == 0xFF)
                {
                    rt_kprintf("[GAMEPAD] All buttons released\n");
                }
            }

            /* 保存当前状态 */
            last_key = key_index;
        }

        /* 定期扫描 */
        rt_thread_mdelay(GAMEPAD_SCAN_INTERVAL_MS);
    }
}

/* ================ 公共API实现 ================ */

/**
 * @brief 启动游戏手柄应用线程
 */
int gamepad_app_start(void)
{
    /* 创建游戏手柄线程 */
    gamepad_thread = rt_thread_create(
        "gamepad",
        gamepad_thread_entry,
        RT_NULL,
        2048,
        RT_THREAD_PRIORITY_MAX / 2,
        10
    );

    if (gamepad_thread == RT_NULL)
    {
        rt_kprintf("[GAMEPAD] Failed to create thread\n");
        return -1;
    }

    /* 启动线程 */
    rt_thread_startup(gamepad_thread);
    rt_kprintf("[GAMEPAD] Application started (scan interval: %dms)\n", GAMEPAD_SCAN_INTERVAL_MS);

    return 0;
}
INIT_APP_EXPORT(gamepad_app_start);

/**
 * @brief 检查游戏手柄是否就绪
 */
bool gamepad_is_ready(void)
{
    return hid_gamepad_is_configured(GAMEPAD_USB_BUS_ID);
}

/**
 * @brief 获取当前按键状态
 */
uint16_t gamepad_get_buttons(void)
{
    return current_buttons;
}
