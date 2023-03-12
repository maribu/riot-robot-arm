#include <assert.h>
#include <stdio.h>

#include "architecture.h"
#include "board.h"
#include "msg.h"
#include "net/gcoap.h"
#include "net/nanocoap.h"
#include "net/nanocoap_sock.h"
#include "periph/adc.h"
#include "shell.h"
#include "tiny_strerror.h"
#include "ztimer.h"

#define MAIN_QUEUE_SIZE (4)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
static WORD_ALIGNED char pad_stack[THREAD_STACKSIZE_DEFAULT];
static nanocoap_sock_t coap_sock;

#ifndef JOYSTICK_ADC_RES
#define JOYSTICK_ADC_RES    ADC_RES_10BIT
#endif

#ifndef SMALL_STEP
#define SMALL_STEP          2
#endif

#ifndef LARGE_STEP
#define LARGE_STEP          16
#endif

#ifndef ROBOT_URL
#define ROBOT_URL           "coap://[fe80::a0bd:b5ff:fe4e:d740]"
#endif

#ifndef PATH_ROTATE
#define PATH_ROTATE         "arm/rotate"
#endif

#ifndef PATH_EXTEND
#define PATH_EXTEND         "arm/extend"
#endif

#ifndef PATH_LIFT
#define PATH_LIFT           "arm/lift"
#endif

#ifndef PATH_GRAB
#define PATH_GRAB           "arm/grab"
#endif

struct joystick_axis {
    adc_t adc_line;
    adc_res_t adc_res;
    int32_t low;
    int32_t high;
    int32_t max;
    int32_t min;
};

static struct joystick_axis axis_x = {
    .adc_line = JOYSTICK_AXIS_X,
    .adc_res = JOYSTICK_ADC_RES,
    .max = 1023,
    .min = 0,
};

static struct joystick_axis axis_y = {
    .adc_line = JOYSTICK_AXIS_Y,
    .adc_res = JOYSTICK_ADC_RES,
    .max = 1023,
    .min = 0,
};

int8_t joystick_read(const struct joystick_axis *axis)
{
    int32_t sample = adc_sample(axis->adc_line, axis->adc_res);
    int32_t result;

    if (sample <= axis->min) {
        return -127;
    }

    if (sample >= axis->max) {
        return 127;
    }

    if (sample < axis->low) {
        result = -127L * (axis->low - sample);
        result /= (axis->low - axis->min);
        return result;
    }

    if (sample > axis->high) {
        result = 127L * (sample - axis->high);
        result /= (axis->max - axis->high);
        return result;
    }

    return 0;
}

int joystick_init(struct joystick_axis *axis)
{
    int res = adc_init(axis->adc_line);
    if (res != 0) {
        return res;
    }

    int32_t neutral = adc_sample(axis->adc_line, axis->adc_res);
    if (neutral == -1) {
        return -1;
    }

    axis->low = neutral - ((neutral - axis->min) >> 5);
    axis->high = neutral + ((axis->max - neutral) >> 5);

    return 0;
}

static void post_movement(const char *path, int8_t step)
{
    char payload[8];
    char response[32];
    int pld_len = snprintf(payload, sizeof(payload) - 1, "%d", (int)step);
    int res = nanocoap_sock_post_non(&coap_sock, path, payload, pld_len,
                                     response, sizeof(response));
    if (res != 0) {
        printf("POST to %s failed: %s\n", path, tiny_strerror(res));
    }
}

static void *pad_thread(void *arg)
{
    (void)arg;
    ztimer_now_t last_weakup = ztimer_now(ZTIMER_MSEC);

    while (1) {
        ztimer_periodic_wakeup(ZTIMER_MSEC, &last_weakup, 100);
        int8_t step_rotate = -((int32_t)LARGE_STEP * joystick_read(&axis_x)) / 128;
        int8_t step_extend = ((int32_t)LARGE_STEP * joystick_read(&axis_y) / 128);
        int8_t step_lift = 0;
        if (!gpio_read(JOYSTICK_BTN_A_PIN)) {
            step_lift = SMALL_STEP;
        }
        if (!gpio_read(JOYSTICK_BTN_C_PIN)) {
            step_lift = -SMALL_STEP;
        }
        int8_t step_grab = 0;
        if (!gpio_read(JOYSTICK_BTN_B_PIN)) {
            step_grab = SMALL_STEP;
        }
        if (!gpio_read(JOYSTICK_BTN_D_PIN)) {
            step_grab = -SMALL_STEP;
        }

        if (step_rotate != 0) {
            post_movement(PATH_ROTATE, step_rotate);
        }

        if (step_extend != 0) {
            post_movement(PATH_EXTEND, step_extend);
        }

        if (step_lift != 0) {
            post_movement(PATH_LIFT, step_lift);
        }

        if (step_grab != 0) {
            post_movement(PATH_GRAB, step_grab);
        }
    }

    return NULL;
}

static void setup_joystick_trhead(void) {
    gpio_init(JOYSTICK_BTN_A_PIN, JOYSTICK_BTN_A_MODE);
    gpio_init(JOYSTICK_BTN_B_PIN, JOYSTICK_BTN_B_MODE);
    gpio_init(JOYSTICK_BTN_C_PIN, JOYSTICK_BTN_C_MODE);
    gpio_init(JOYSTICK_BTN_D_PIN, JOYSTICK_BTN_D_MODE);
    gpio_init(JOYSTICK_BTN_E_PIN, JOYSTICK_BTN_E_MODE);
    gpio_init(JOYSTICK_BTN_F_PIN, JOYSTICK_BTN_F_MODE);
    gpio_init(JOYSTICK_BTN_K_PIN, JOYSTICK_BTN_K_MODE);
    joystick_init(&axis_x);
    joystick_init(&axis_y);
    int res = nanocoap_sock_url_connect(ROBOT_URL, &coap_sock);
    if (res != 0) {
        printf("connect to %s failed: %s\n", ROBOT_URL, tiny_strerror(res));
        return;
    }
    thread_create(pad_stack, sizeof(pad_stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                  pad_thread, NULL, "pad");

}

int main(void)
{
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    setup_joystick_trhead();
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
