#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
  uint16_t event;
  uint16_t len;
  uint16_t offset;
  uint16_t layer_specific;
  uint8_t data[];
} BT_HDR;

#define BT_HDR_SIZE (sizeof(BT_HDR))

typedef uint16_t command_opcode_t;

uint8_t* read_command_complete_header(uint8_t *stream,
                                      command_opcode_t expected_opcode,
                                      size_t minimum_bytes_after);

#define STREAM_SKIP_UINT8(p) \
  do {                       \
    (p) += 1;                \
  } while (0)
#define STREAM_SKIP_UINT16(p) \
  do {                        \
    (p) += 2;                 \
  } while (0)

#define STREAM_TO_INT8(u8, p) \
  {                           \
    (u8) = (*((int8_t*)p));   \
    (p) += 1;                 \
  }
#define STREAM_TO_UINT8(u8, p) \
  {                            \
    (u8) = (uint8_t)(*(p));    \
    (p) += 1;                  \
  }
#define STREAM_TO_UINT16(u16, p)                                  \
  {                                                               \
    (u16) = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); \
    (p) += 2;                                                     \
  }
#define STREAM_TO_UINT24(u32, p)                                      \
  {                                                                   \
    (u32) = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + \
             ((((uint32_t)(*((p) + 2)))) << 16));                     \
    (p) += 3;                                                         \
  }
#define STREAM_TO_UINT32(u32, p)                                      \
  {                                                                   \
    (u32) = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + \
             ((((uint32_t)(*((p) + 2)))) << 16) +                     \
             ((((uint32_t)(*((p) + 3)))) << 24));                     \
    (p) += 4;                                                         \
  }

#define STREAM_TO_ARRAY(a, p, len)                                   \
  {                                                                  \
    int ijk;                                                         \
    for (ijk = 0; ijk < (len); ijk++) ((uint8_t*)(a))[ijk] = *(p)++; \
  }

#define UINT32_TO_STREAM(p, u32)     \
  {                                  \
    *(p)++ = (uint8_t)(u32);         \
    *(p)++ = (uint8_t)((u32) >> 8);  \
    *(p)++ = (uint8_t)((u32) >> 16); \
    *(p)++ = (uint8_t)((u32) >> 24); \
  }
#define UINT24_TO_STREAM(p, u24)     \
  {                                  \
    *(p)++ = (uint8_t)(u24);         \
    *(p)++ = (uint8_t)((u24) >> 8);  \
    *(p)++ = (uint8_t)((u24) >> 16); \
  }
#define UINT16_TO_STREAM(p, u16)    \
  {                                 \
    *(p)++ = (uint8_t)(u16);        \
    *(p)++ = (uint8_t)((u16) >> 8); \
  }
#define UINT8_TO_STREAM(p, u8) \
  { *(p)++ = (uint8_t)(u8); }


#define BE_STREAM_TO_UINT8(u8, p) \
  {                               \
    (u8) = (uint8_t)(*(p));       \
    (p) += 1;                     \
  }
#define BE_STREAM_TO_UINT16(u16, p)                                       \
  {                                                                       \
    (u16) = (uint16_t)(((uint16_t)(*(p)) << 8) + (uint16_t)(*((p) + 1))); \
    (p) += 2;                                                             \
  }
#define BE_STREAM_TO_UINT24(u32, p)                                     \
  {                                                                     \
    (u32) = (((uint32_t)(*((p) + 2))) + ((uint32_t)(*((p) + 1)) << 8) + \
             ((uint32_t)(*(p)) << 16));                                 \
    (p) += 3;                                                           \
  }
#define BE_STREAM_TO_UINT32(u32, p)                                      \
  {                                                                      \
    (u32) = ((uint32_t)(*((p) + 3)) + ((uint32_t)(*((p) + 2)) << 8) +    \
             ((uint32_t)(*((p) + 1)) << 16) + ((uint32_t)(*(p)) << 24)); \
    (p) += 4;                                                            \
  }

#define HCI_COMMAND_COMPLETE_EVT 0x0E

#define CHECK(...) assert(__VA_ARGS__)

typedef enum {
    HCI_VS_QBCE_READ_LOCAL_QLM_SUPPORTED_FEATURES  = 0x09,
    HCI_VS_QBCE_READ_REMOTE_QLM_SUPPORTED_FEATURES = 0x0A,
    HCI_VS_QBCE_READ_LOCAL_QLL_SUPPORTED_FEATURES  = 0x0B,
    HCI_VS_QBCE_READ_REMOTE_QLL_SUPPORTED_FEATURES = 0x0C,
}  __attribute__ ((__packed__)) qbce_cmd_opcode_t;

typedef enum {
    HCI_SUCCESS = 0x00,
    HCI_ERR_UNKNOWN_HCI_COMMAND = 0x01,
    HCI_ERR_UNKNOWN_CONNECTION_IDENTIFIER = 0x02,
    HCI_ERR_HARDWARE_FAILURE = 0x03,
    HCI_ERR_PAGE_TIMEOUT = 0x04,
    HCI_ERR_AUTHENTICATION_FAILURE = 0x05,
    HCI_ERR_KEY_MISSING = 0x06,
    HCI_ERR_MEMORY_FULL = 0x07,
    HCI_ERR_CONNECTION_TIMEOUT = 0x08,
    HCI_ERR_MAX_NUM_CONNECTIONS = 0x09,
    HCI_ERR_MAX_NUM_SCO_CONNECTIONS = 0x0A,
    HCI_ERR_CONNECTION_ALREADY_EXISTS = 0x0B,
    HCI_ERR_COMMAND_DISALLOWED = 0x0C,
    HCI_ERR_CONN_REJECTED_RESOURCES = 0x0D,
    HCI_ERR_CONN_REJECTED_SECURITY = 0x0E,
    HCI_ERR_CONN_REJECTED_UNACCEPTABLE_BDADDR = 0x0F,
    HCI_ERR_HOST_TIMEOUT = 0x10,
    HCI_ERR_UNSUPPORTED_FEATURE_PARAM = 0x11,
    HCI_ERR_INVALID_HCI_COMMAND_PARAM = 0x12,
    HCI_ERR_OE_TERM_CONN_USER = 0x13,
    HCI_ERR_OE_TERM_CONN_LOW_RESOURCES = 0x14,
    HCI_ERR_OE_TERM_CONN_POWER_OFF = 0x15,
    HCI_ERR_LH_CONN_TERMINATED = 0x16,
    HCI_ERR_REPEATED_ATTEMPTS = 0x17,
    HCI_ERR_PAIRING_NOT_ALLOWED = 0x18,
    HCI_ERR_UNKNOWN_LMP_PDU = 0x19,
    HCI_ERR_UNSUPPORTED_FEATURE = 0x1A,
    HCI_ERR_SCO_OFFSET_REJECTED = 0x1B,
    HCI_ERR_SCO_INTERVAL_REJECTED = 0x1C,
    HCI_ERR_AIR_MODE_REJECTED = 0x1D,
    HCI_ERR_INVALID_LMP_PARAM = 0x1E,
    HCI_ERR_UNSPECIFIED = 0x1F,
    HCI_ERR_UNSUPPORTED_LMP_PARAM_VALUE = 0x20,
    HCI_ERR_SWITCH_NOT_ALLOWED = 0x21,
    HCI_ERR_LMP_RESPONSE_TIMEOUT = 0x22,
    HCI_ERR_LMP_ERROR_TRANS_COLLISION = 0x23,
    HCI_ERR_PDU_NOT_ALLOWED = 0x24,
    HCI_ERR_ENCRYPTION_MODE_NOT_ACCEPTABLE = 0x25,
    HCI_ERR_UNIT_KEY_USED = 0x26,
    HCI_ERR_QOS_NOT_SUPPORTED = 0x27,
    HCI_ERR_INSTANT_PASSED = 0x28,
    HCI_ERR_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED = 0x29,
    HCI_ERR_DIFFERENT_TRANSACTION_COLLISION = 0x2A,
    HCI_ERR_RESERVED1 = 0x2B,
    HCI_ERR_QOS_UNACCEPTABLE_PARAMETER = 0x2C,
    HCI_ERR_QOS_REJECTED = 0x2D,
    HCI_ERR_CHANNEL_CLASSIFICATION_NOT_SUPPORTED = 0x2E,
    HCI_ERR_INSUFFICIENT_SECURITY = 0x2F,
    HCI_ERR_PARAMETER_OUT_OF_MANDATORY_RANGE = 0x30,
    HCI_ERR_RESERVED2 = 0x31,
    HCI_ERR_ROLE_SWITCH_PENDING = 0x32,
    HCI_ERR_RESERVED3 = 0x33,
    HCI_ERR_RESERVED_SLOT_VIOLATION = 0x34,
    HCI_ERR_ROLE_SWITCH_FAILED = 0x35,
    HCI_ERR_INQUIRY_RESPONSE_DATA_TOO_LARGE = 0x36,
    HCI_ERR_SIMPLE_PAIRING_NOT_SUPPORTED_BY_HOST = 0x37,
    HCI_ERR_HOST_BUSY_PAIRING = 0x38,
    HCI_ERR_CONTROLLER_BUSY = 0x3A,
    HCI_ERR_LE_UNACCEPTABLE_CONNECTION_PARAMS =0x3B,
    HCI_ERR_LE_ADVERTISING_TIMEOUT = 0x3C,
    HCI_ERR_LE_MIC_FAILURE = 0x3D,
    HCI_ERR_LE_CONN_FAILED_TO_ESTABLISH = 0x3E,
    HCI_ERR_PAL_MAC_CONN_TIMEOUT = 0x3F,
    HCI_ERR_COARSE_ADJ_REJECTED_WILL_DRAG = 0x40,
    HCI_ERR_LE_UNKNOWN_ADVERTISING_ID = 0x42,
    HCI_ERR_LIMIT_REACHED = 0x43,
    HCI_ERR_OPERATION_CANCELLED_BY_HOST = 0x44,
    HCI_ERR_PACKET_TOO_LONG = 0x45,
} __attribute__ ((__packed__)) hci_status_t;

static_assert(sizeof(qbce_cmd_opcode_t) == 1, "Enum size assumtion is incorrect");

#define OGF_VS 0x3F

#define OCF_VS_QBCE 0x0051
#define OCF_VS_ADDON 0x001D

#define HCI_VS_QBCE_OCF (OCF_VS_QBCE | (OGF_VS << 10))

#define HCI_VS_GET_ADDON_FEATURES_SUPPORT (OCF_VS_ADDON | (OGF_VS << 10))
