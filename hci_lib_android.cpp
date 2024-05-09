#include <android/hardware/bluetooth/1.0/IBluetoothHci.h>
#include <android/hardware/bluetooth/1.0/IBluetoothHciCallbacks.h>
#include <android/hardware/bluetooth/1.0/types.h>
#include <android/hardware/bluetooth/1.1/IBluetoothHci.h>
#include <android/hardware/bluetooth/1.1/IBluetoothHciCallbacks.h>
#include <hwbinder/ProcessState.h>
#include <hwbinder/IPCThreadState.h>

#include "hci_parser.h"
#include "hci_lib_android.h"


#include <iostream>
#include <cstdio>
#include <queue>
#include <memory>

using std::cout;
using std::endl;
using std::queue;
using std::unique_ptr;
using std::make_unique;

using android::hardware::IPCThreadState;
using android::hardware::bluetooth::V1_0::HciPacket;
using android::hardware::bluetooth::V1_0::Status;
using android::hardware::ProcessState;
using android::hardware::Return;
using android::hardware::Void;
using android::hardware::hidl_vec;
using namespace ::android::hardware::bluetooth;
static android::sp<V1_1::IBluetoothHci> btHci_1_1;

#define LOG_INFO(tag, fmt, ...) printf("INFO: %s:" fmt "\n", tag, __VA_ARGS__)
#define LOG_ERROR(tag, fmt, ...) printf("ERROR: %s:" fmt "\n", tag, __VA_ARGS__)

#define LOG_TAG "qhs-util"

class BluetoothPacketQueue {
public:
    BluetoothPacketQueue() : mEvents(), mAcl(), mSco() {}

    bool hasEvent() {
        return !mEvents.empty();
    }

    BT_HDR* getEvent() {
        if (mEvents.empty()) {
            return nullptr;
        }
        cout << "Queue empty: " << mEvents.empty() << endl;
        auto p = mEvents.front();
        mEvents.pop();
        return p;
    }

    BT_HDR* getAcl() {
        auto p = mAcl.front();
        mAcl.pop();
        return p;
    }

    BT_HDR* getSco() {
        auto p = mSco.front();
        mSco.pop();
        return p;
    }

    void putEvent(BT_HDR* packet) {
        mEvents.push(packet);
    }

    void putAcl(BT_HDR* packet) {
        mAcl.push(packet);
    }

    void putSco(BT_HDR* packet) {
        mSco.push(packet);
    }
private:
    queue<BT_HDR*> mEvents;
    queue<BT_HDR*> mAcl;
    queue<BT_HDR*> mSco;
};

static BluetoothPacketQueue pq;

std::atomic<bool> initialization_complete = false;

class BluetoothHciCallbacks : public V1_1::IBluetoothHciCallbacks {
 public:
  BluetoothHciCallbacks() {
  }

  BT_HDR* WrapPacketAndCopy(uint16_t event, const hidl_vec<uint8_t>& data) {
    size_t packet_size = data.size() + BT_HDR_SIZE;
    BT_HDR* packet = (BT_HDR *) malloc(packet_size);
    packet->offset = 0;
    packet->len = data.size();
    packet->layer_specific = 0;
    packet->event = event;
    // TODO(eisenbach): Avoid copy here; if BT_HDR->data can be ensured to
    // be the only way the data is accessed, a pointer could be passed here...
    memcpy(packet->data, data.data(), data.size());
    return packet;
  }

  Return<void> initializationComplete(Status status) {

    if(status == Status::SUCCESS) {
      LOG_INFO(LOG_TAG, "%s: HCI Init OK", __func__);
      initialization_complete = true;
    } else {
      LOG_ERROR(LOG_TAG, "%s: HCI Init failed ", __func__);
    }
    return Void();
  }

  Return<void> hciEventReceived(const hidl_vec<uint8_t>& event) {
    auto packet = WrapPacketAndCopy(MSG_HC_TO_STACK_HCI_EVT, event);
    pq.putEvent(packet);
    return Void();
  }

  Return<void> aclDataReceived(const hidl_vec<uint8_t>& data) {
    auto packet = WrapPacketAndCopy(MSG_HC_TO_STACK_HCI_ACL, data);
    pq.putAcl(packet);
    return Void();
  }

  Return<void> scoDataReceived(const hidl_vec<uint8_t>& data) {
    auto packet = WrapPacketAndCopy(MSG_HC_TO_STACK_HCI_SCO, data);
    pq.putSco(packet);
    return Void();
  }

  Return<void> isoDataReceived(const hidl_vec<uint8_t>& data) {
    /* customized change based on the requirements */
    LOG_INFO(LOG_TAG, "%s", __func__);
    return Void();
  }
};

int hci_devba(int dev_id, bdaddr_t *addr) {
	memset(addr, 0, sizeof(*addr));
	return 0;
}

int hci_open_dev(int dev_id) {
    if (btHci_1_1 == nullptr) {
        LOG_ERROR(LOG_TAG, "%s: HIDL daemon is dead", __func__);
        return -1;
    }

    android::sp<V1_1::IBluetoothHciCallbacks> callbacks = new BluetoothHciCallbacks();
    auto hidl_daemon_status = btHci_1_1->initialize_1_1(callbacks);

    if(!hidl_daemon_status.isOk()) {
        LOG_ERROR(LOG_TAG, "%s: HIDL daemon is dead", __func__);
        btHci_1_1 = nullptr;
        return -1;
    }
	return 0;
}

int hci_close_dev(int dd) {
    auto hidl_daemon_status = btHci_1_1->close();
    if(!hidl_daemon_status.isOk()) {
        LOG_ERROR(LOG_TAG, "%s: HIDL daemon is dead", __func__);
        return -1;
    }
    btHci_1_1 = nullptr;
	return 0;
}

int hci_devid(const char *name) {
    btHci_1_1 = V1_1::IBluetoothHci::getService();

    if (btHci_1_1 != nullptr) {
        LOG_INFO(LOG_TAG, "%s: Using IBluetoothHci 1.1 service", __func__);
    } else {
        LOG_ERROR(LOG_TAG, "%s: Failed to get IBluetooth service", __func__);
    }
	return 42;
}

static BT_HDR* make_packet(size_t data_size) {
  BT_HDR* ret = (BT_HDR*) malloc(sizeof(BT_HDR) + data_size);
  assert(ret != NULL && "Buy more RAM lol");
  ret->event = 0;
  ret->offset = 0;
  ret->layer_specific = 0;
  ret->len = data_size;
  return ret;
}

static BT_HDR* make_command(uint16_t opcode, size_t parameter_size,
                            uint8_t** stream_out) {
  BT_HDR* packet = make_packet(HCI_COMMAND_PREAMBLE_SIZE + parameter_size);

  uint8_t* stream = packet->data;
  UINT16_TO_STREAM(stream, opcode);
  UINT8_TO_STREAM(stream, parameter_size);

  if (stream_out != NULL) *stream_out = stream;

  return packet;
}

static BT_HDR* make_command_no_params(uint16_t opcode) {
  return make_command(opcode, 0, NULL);
}

int hci_send_cmd(int dd, uint16_t ogf, uint16_t ocf, size_t len, uint8_t *buf) {
    HciPacket data;

    if(btHci_1_1 == nullptr) {
      LOG_INFO(LOG_TAG, "%s: Link with Bluetooth HIDL service is closed", __func__);
      return -1;
    }

    uint8_t* stream_out = NULL;

    auto packet = make_command((ogf << 10) | ocf, len, &stream_out);

    assert(stream_out != NULL && "Error in make_command");
    if (len > 0) memcpy(stream_out, buf, len);

    cout << __func__ << ": OPCODE: " << ((ogf << 10) | ocf) << endl;

    data.setToExternal(packet->data + packet->offset, packet->len);

    auto hidl_daemon_status = btHci_1_1->sendHciCommand(data);
    if(!hidl_daemon_status.isOk()) {
        LOG_ERROR(LOG_TAG, "%s: send Command failed, HIDL daemon is dead", __func__);
        return -1;
    }
	return 0;
}

void hci_read_local_version(int dd, struct hci_version *ver, size_t timeout) {
    hci_send_cmd(dd, 0x04, 0x0001, 0, NULL);

    uint8_t buf[HCI_MAX_EVENT_SIZE];
    int len = hci_read(dd, buf, HCI_MAX_EVENT_SIZE);

    printf("%s: ", __func__);
    for (int i = 0; i < len; i++) {
        printf("0x%02x, ", buf[i]);
    }
    printf("\n");

    uint8_t *data = buf + 6;
    //0x0e, 0x0c, 0x01, 0x01, 0x10, 0x00, 0x0c, 0x00, 0x00, 0x0c, 0x1d, 0x00, 0x7b, 0x58,
    //LEN                    STATUS HVER  HCI_REV    LVER    MANUFAC     LSUBVER

    STREAM_TO_UINT8(ver->hci_ver, data);
    STREAM_TO_UINT16(ver->hci_rev, data);
    STREAM_TO_UINT8(ver->lmp_ver, data);
    STREAM_TO_UINT16(ver->manufacturer, data);
    STREAM_TO_UINT16(ver->lmp_subver, data);
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int hci_read(int dd, void *buf, size_t size) {
    while(!pq.hasEvent()) {
        // Waiting...
        //cout << pq.hasEvent();
        usleep(10);
    }
    cout << "pq.hasEvent(): " << pq.hasEvent() << endl;

    auto packet = pq.getEvent();
    cout << "packet: " << packet << ", len: " << packet->len << endl;
    memcpy(buf, packet->data, MIN(size, packet->len));
    free(packet);
    return MIN(size, packet->len);
}
