#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "hci_parser.h"

static const command_opcode_t NO_OPCODE_CHECKING = 0;

uint8_t* read_command_complete_header(uint8_t *stream,
                                      command_opcode_t expected_opcode,
                                      size_t minimum_bytes_after) {
  // Read the event header
  uint8_t event_code;
  uint8_t parameter_length;
  STREAM_TO_UINT8(event_code, stream);
  STREAM_TO_UINT8(parameter_length, stream);

  const size_t parameter_bytes_we_read_here = 4;

  // Check the event header values against what we expect
  CHECK(event_code == HCI_COMMAND_COMPLETE_EVT);
  CHECK(parameter_length >=
        (parameter_bytes_we_read_here + minimum_bytes_after));

  // Read the command complete header
  command_opcode_t opcode;
  uint8_t status;
  STREAM_SKIP_UINT8(stream);  // skip the number of hci command packets field
  STREAM_TO_UINT16(opcode, stream);

  // Check the command complete header values against what we expect
  if (expected_opcode != NO_OPCODE_CHECKING) {
    CHECK(opcode == expected_opcode);
  }

  // Assume the next field is the status field
  STREAM_TO_UINT8(status, stream);

  if (status != HCI_SUCCESS) {
    fprintf(stderr, "%s: return status - 0x%x\n", __func__, status);
    return NULL;
  }

  return stream;
}
