
#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_pwm.h"
#include "iot_i2c.h"
#include "iot_errno.h"

#include "ssd1306.h"

#include "hi_io.h"
#include "lvgl.h"

#include "lv_port_disp_template.h"
#define OLED_I2C_BAUDRATE 400 * 1000

static void gui_label_create(void);

// 按键组件
void lv_ex_label(void)
{
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn, 0, 0); // x,y
    lv_obj_set_size(btn, 60, 30);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "FSR");
    lv_obj_center(label);
}

void TestGetTick(void)
{
    for (int i = 0; i < 20; i++)
    {
        usleep(10 * 1000);
        printf("HAL_GetTick(): %d\r\n", HAL_GetTick());
    }

    for (int i = 0; i < 20; i++)
    {
        HAL_Delay(25);
        printf(" HAL_GetTick(): %d\r\n", HAL_GetTick());
    }
}

void LVGLTestTask(void *arg)
{
    // IO口初始化
    (void)arg;
    IoTGpioInit(HI_IO_NAME_GPIO_13);
    IoTGpioInit(HI_IO_NAME_GPIO_14);

    hi_io_set_func(HI_IO_NAME_GPIO_13, HI_IO_FUNC_GPIO_13_I2C0_SDA);
    hi_io_set_func(HI_IO_NAME_GPIO_14, HI_IO_FUNC_GPIO_14_I2C0_SCL);

    IoTI2cInit(0, OLED_I2C_BAUDRATE);

    // WatchDogDisable();

    usleep(20 * 1000);
    // ssd1306初始化
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("Hello HarmonyOS!", Font_7x10, White);

    uint32_t start = HAL_GetTick();
    ssd1306_UpdateScreen();
    uint32_t end = HAL_GetTick();
    printf("ssd1306_UpdateScreen time cost: %d ms.\r\n", end - start);

    lv_init();           // lvgl初始化
    lv_port_disp_init(); // lvgl屏幕驱动初始化
    // lv_ex_label();       // 按钮组件
    printf("LVGLTestTask start.\r\n");

    gui_label_create();
    // 修改while循环中的延时参数
    while (1)
    {
        lv_tick_inc(10); // 与刷新率匹配
        lv_task_handler();
        usleep(10000); // 调整为10ms刷新周期（100Hz）
    }
}

static void gui_label_create(void)
{
    lv_obj_t *label1 = lv_label_create(lv_scr_act());
    lv_label_set_text(label1, "lvgl");
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_14, LV_STATE_DEFAULT); // 使用14px字体
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);                                  // 移除垂直偏移
}

// 线程创建
void LVGLTestDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "LVGL_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 20; // 内存分配一定要充足
    attr.priority = osPriorityNormal;

    if (osThreadNew(LVGLTestTask, NULL, &attr) == NULL)
    {
        printf("[LVGL_Task] Falied to create LVGL_Task!\n");
    }
}
APP_FEATURE_INIT(LVGLTestDemo);
