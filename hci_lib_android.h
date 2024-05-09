#pragma once

#include "bt_compidstr.h"
#include <atomic>

extern std::atomic<bool> initialization_complete;

#define HCI_MAX_EVENT_SIZE 256
#define MSG_HC_TO_STACK_HCI_ERR 0x1300      /* eq. BT_EVT_TO_BTU_HCIT_ERR */
#define MSG_HC_TO_STACK_HCI_ACL 0x1100      /* eq. BT_EVT_TO_BTU_HCI_ACL */
#define MSG_HC_TO_STACK_HCI_SCO 0x1200      /* eq. BT_EVT_TO_BTU_HCI_SCO */
#define MSG_HC_TO_STACK_HCI_EVT 0x1000      /* eq. BT_EVT_TO_BTU_HCI_EVT */

#define hci_filter_all_events(...)
#define hci_filter_set_ptype(...)
#define hci_filter_clear(...)

// 2 bytes for opcode, 1 byte for parameter length (Volume 2, Part E, 5.4.1)
#define HCI_COMMAND_PREAMBLE_SIZE 3
// 2 bytes for handle, 2 bytes for data length (Volume 2, Part E, 5.4.2)
#define HCI_ACL_PREAMBLE_SIZE 4
// 2 bytes for handle, 1 byte for data length (Volume 2, Part E, 5.4.3)
#define HCI_SCO_PREAMBLE_SIZE 3
// 1 byte for event code, 1 byte for parameter length (Volume 2, Part E, 5.4.4)
#define HCI_EVENT_PREAMBLE_SIZE 2

struct hci_filter {
	int dummy;
};

typedef struct {
	uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

struct hci_version {
	uint16_t manufacturer;
	uint8_t  hci_ver;
	uint16_t hci_rev;
	uint8_t  lmp_ver;
	uint16_t lmp_subver;
};
int hci_devid(const char *name);
int hci_devba(int dev_id, bdaddr_t *addr);
int hci_open_dev(int dev_id);
int hci_close_dev(int dd);
int hci_send_cmd(int dd, uint16_t ogf, uint16_t ocf, size_t len, uint8_t *buf);
void hci_read_local_version(int dd, struct hci_version *ver, size_t timeout);
int hci_read(int fd, void *buf, size_t size);
