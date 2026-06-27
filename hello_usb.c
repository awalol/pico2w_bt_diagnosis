#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "pico/version.h"

#define INQUIRY_DURATION_1280MS_UNITS 8
#define RESCAN_DELAY_MS 5000

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

static btstack_packet_callback_registration_t hci_event_callback_registration;
static uint32_t scan_round;
static uint32_t devices_found;
static bool scanning;
static bool rescan_pending;
static absolute_time_t next_scan_time;

static void start_scan(void);

static void wait_for_usb_stdio(void) {
    absolute_time_t timeout = make_timeout_time_ms(5000);

    while (!stdio_usb_connected() && absolute_time_diff_us(get_absolute_time(), timeout) > 0) {
        sleep_ms(100);
    }
}

static void print_basic_info(void) {
    char board_id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
    pico_get_unique_board_id_string(board_id, sizeof(board_id));

    printf("pico2w_bt_diagnosis\n");
#ifdef PICO_BOARD
    printf("Board: %s\n", STR(PICO_BOARD));
#endif
    printf("Pico SDK: %s\n", PICO_SDK_VERSION_STRING);
    printf("Board ID: %s\n", board_id);
    fflush(stdout);
}

static void print_inquiry_result(uint8_t *packet) {
    bd_addr_t addr;
    gap_event_inquiry_result_get_bd_addr(packet, addr);
    devices_found++;

    printf("[%lu] device %lu: addr=%s, cod=0x%06lx",
           (unsigned long) scan_round,
           (unsigned long) devices_found,
           bd_addr_to_str(addr),
           (unsigned long) gap_event_inquiry_result_get_class_of_device(packet));

    if (gap_event_inquiry_result_get_rssi_available(packet)) {
        printf(", rssi=%d dBm", (int8_t) gap_event_inquiry_result_get_rssi(packet));
    }

    if (gap_event_inquiry_result_get_name_available(packet)) {
        char name[241];
        uint8_t name_len = gap_event_inquiry_result_get_name_len(packet);
        if (name_len >= sizeof(name)) {
            name_len = sizeof(name) - 1;
        }
        memcpy(name, gap_event_inquiry_result_get_name(packet), name_len);
        name[name_len] = 0;
        printf(", name=\"%s\"", name);
    }

    printf("\n");
    fflush(stdout);
}

static void schedule_rescan(void) {
    next_scan_time = make_timeout_time_ms(RESCAN_DELAY_MS);
    rescan_pending = true;
    printf("Next scan in %u ms.\n", RESCAN_DELAY_MS);
    fflush(stdout);
}

static void start_scan(void) {
    if (scanning) {
        return;
    }

    scan_round++;
    devices_found = 0;
    scanning = true;

    printf("\nScan %lu started\n", (unsigned long) scan_round);
    fflush(stdout);

    int status = gap_inquiry_start(INQUIRY_DURATION_1280MS_UNITS);
    if (status != 0) {
        scanning = false;
        printf("Failed to start scan: 0x%02x\n", status);
        fflush(stdout);
        schedule_rescan();
    }
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    switch (hci_event_packet_get_type(packet)) {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                printf("Bluetooth ready\n");
                fflush(stdout);
                start_scan();
            }
            break;

        case GAP_EVENT_INQUIRY_RESULT:
            print_inquiry_result(packet);
            break;

        case GAP_EVENT_INQUIRY_COMPLETE:
            scanning = false;
            printf("Scan %lu complete: %lu device(s)\n",
                   (unsigned long) scan_round,
                   (unsigned long) devices_found);
            fflush(stdout);
            schedule_rescan();
            break;

        default:
            break;
    }
}

int main(void) {
    stdio_init_all();
    wait_for_usb_stdio();
    print_basic_info();

    printf("Initializing CYW43/BTstack...\n");
    fflush(stdout);

    int init_status = cyw43_arch_init();
    printf("CYW43 init status: %d\n", init_status);
    fflush(stdout);

    if (init_status != 0) {
        while (true) {
            sleep_ms(1000);
        }
    }

    hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);

    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    printf("Powering on Bluetooth...\n");
    fflush(stdout);
    hci_power_control(HCI_POWER_ON);

    while (true) {
        if (rescan_pending && absolute_time_diff_us(get_absolute_time(), next_scan_time) <= 0) {
            rescan_pending = false;
            start_scan();
        }

        sleep_ms(100);
    }
}
