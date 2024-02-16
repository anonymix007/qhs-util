#include "bt_compidstr.h"

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

#define HCI_MAX_EVENT_SIZE 256

struct hci_filter {
	int dummy;
};

int hci_devba(int dev_id, bdaddr_t *addr) {
	memset(addr, 0, sizeof(*addr));
	return 0;
}

int hci_open_dev(int dev_id) {
	return -1;
}
int hci_close_dev(int dd) {
	return -1;	
}
int hci_devid(const char *name) {
	return 0;
}
#define hci_filter_all_events(...)
#define hci_filter_set_ptype(...)
#define hci_filter_clear(...)

int hci_send_cmd(int dd, uint16_t ogf, uint16_t ocf, size_t len, uint8_t *buf) {
	return 0;
}
#define hci_read_local_version(...)
