#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "hci_parser.c"

#define DEBUG

#define ARRAY_SIZE(x) sizeof(x) / sizeof((x)[0])
#define BOOL(x) (x) ? "T" : "F"

#define BDADDR_Fmt "%02X:%02X:%02X:%02X:%02X:%02X"
#define BDADDR_Arg(a) (a).b[5], (a).b[4], (a).b[3], (a).b[2], (a).b[1], (a).b[0]

void hexdump(const char *start, uint8_t *buf, size_t len) {
    printf("%s0x%02x", start, buf[0]);
    for(size_t i = 1; i < len; i++) {
        printf(", 0x%02x", buf[i]);
    }
    printf("\n");
}

typedef struct {
     /*! Company/vendor specific id. */
    uint16_t comp_id;
    /*! Minimum lmp version for sc/qhs support. */
    uint8_t min_lmp_version;
    /*! Minimum lmp sub version for sc/qhs support. */
    uint16_t min_lmp_sub_version;
} vendor_info_t;


vendor_info_t versions[] = {
    {0x001D, 9, 2000},
    {0x000A, 9, 3000},
};

typedef struct {
    uint8_t split_acl : 1;
    uint8_t tws_esco : 1;
    uint8_t esco_dtx : 1;
    uint8_t high_level_ch_msg : 1;
    uint8_t bredr_qhs_p2 : 1;
    uint8_t qhs_p3 : 1;
    uint8_t qhs_p4 : 1;
    uint8_t qhs_p5 : 1;

    uint8_t qhs_p6 : 1;
    uint8_t rtp_burst : 1;
    uint8_t frozen_clk : 1;
    uint8_t rt_soft_comb : 1;
    uint8_t nonce : 1;
    uint8_t reserved1 : 3;

    uint8_t reserved2[14];
}  __attribute__ ((__packed__)) qlmp_feature_set_t;

#define BOOL_Fmt(x) "\033[%dm" x "\033[39m"
#define BOOL_Arg(x) (x) ? 32 : 31

#define QLMP_FEATURE_SET_Fmt "QHS: ["BOOL_Fmt("2M/BR/EDR")", "BOOL_Fmt("3M")", "BOOL_Fmt("4M")", "BOOL_Fmt("5M")", "BOOL_Fmt("6M")"], "BOOL_Fmt("Higher Layer Channel Messages")",\n    " \
                             BOOL_Fmt("eSCO DTX")", "BOOL_Fmt("TWS eSCO")", "BOOL_Fmt("Split ACL")",\n    " \
                             BOOL_Fmt("BR/EDR Packet Emulation Mode separate ACL and eSCO nonce support")",\n    " \
                             BOOL_Fmt("Real Time Soft Combining")", "BOOL_Fmt("Frozen CLK eSCO Nonce Format")",\n    " \
                             BOOL_Fmt("Round Trip Phase measurement burst support")

#define QLMP_FEATURE_SET_Arg(x) BOOL_Arg((x).bredr_qhs_p2), BOOL_Arg((x).qhs_p3), BOOL_Arg((x).qhs_p4), BOOL_Arg((x).qhs_p5), BOOL_Arg((x).qhs_p6), \
                                BOOL_Arg((x).high_level_ch_msg), BOOL_Arg((x).esco_dtx), BOOL_Arg((x).tws_esco), BOOL_Arg((x).split_acl), \
                                BOOL_Arg((x).nonce), BOOL_Arg((x).rt_soft_comb), BOOL_Arg((x).frozen_clk), BOOL_Arg((x).rtp_burst)


typedef struct {
    uint8_t status;
    qbce_cmd_opcode_t opcode;
    union {
        qlmp_feature_set_t qlmp;
    };
} __attribute__ ((packed)) qbce_event_t;

int hci_read_local_qlmp_features(int dd, qlmp_feature_set_t *qlmp, int to) {
    uint8_t buf[HCI_MAX_EVENT_SIZE] = {HCI_VS_QBCE_READ_LOCAL_QLM_SUPPORTED_FEATURES};
    if (hci_send_cmd(dd, OGF_VS, OCF_VS_QBCE, 1, buf) < 0) {
        perror("Error reading local QLMP features");
        return -1;
    }

    ssize_t len = 0;
    if ((len = read(dd, buf, sizeof(buf))) < 0) {
        perror("Read failed");
        return -1;
    }

#ifdef DEBUG

    printf("HCI QLMP Features (len %zd)", len);
    hexdump(": ", buf, len);

#endif

    uint8_t* stream = read_command_complete_header(&buf[1], HCI_VS_QBCE_OCF, sizeof(*qlmp));

   if (stream) {
     uint8_t sub_opcode;
     STREAM_TO_UINT8(sub_opcode, stream);
     if (sub_opcode == HCI_VS_QBCE_READ_LOCAL_QLM_SUPPORTED_FEATURES) {
         uint8_t *arr = (void *) qlmp;
         STREAM_TO_ARRAY(arr, stream, sizeof(*qlmp));
     }
   } else {
     fprintf(stderr, "%s: stream null check cmnd status reason", __func__);
     return -1;
   }
    return 0;
}

typedef struct {
    uint8_t qll_hs_p2_tx : 1;
    uint8_t qll_hs_p3_tx : 1;
    uint8_t qll_hs_p4_tx : 1;
    uint8_t qll_hs_p5_tx : 1;
    uint8_t qll_hs_p6_tx : 1;
    uint8_t qll_hs_p2_rx : 1;
    uint8_t qll_hs_p3_rx : 1;
    uint8_t qll_hs_p4_rx : 1;

    uint8_t qll_hs_p5_rx : 1;
    uint8_t qll_hs_p6_rx : 1;
    uint8_t qll_hs_f2_tx : 1;
    uint8_t qll_hs_f3_tx : 1;
    uint8_t qll_hs_f4_tx : 1;
    uint8_t qll_hs_f5_tx : 1;
    uint8_t qll_hs_f6_tx : 1;
    uint8_t qll_hs_f2_rx : 1;

    uint8_t qll_hs_f3_rx  : 1;
    uint8_t qll_hs_f4_rx  : 1;
    uint8_t qll_hs_f5_rx  : 1;
    uint8_t qll_hs_f6_rx  : 1;
    uint8_t rtcs          : 1;
    uint8_t reserved1     : 1;
    uint8_t qll_ext_iso   : 1;
    uint8_t qll_ext_isoal : 1;

    uint8_t qll_l_edph          : 1;
    uint8_t reserved3           : 1;
    uint8_t qll_ft_change       : 1;
    uint8_t qll_bn_var_qhs_rate : 1;
    uint8_t reserved2           : 4;

    uint8_t reserved4[3];

    uint8_t reserved6 : 5;
    uint8_t qll_xpan  : 1;
    uint8_t reserved5 : 2;
}  __attribute__ ((__packed__)) qll_feature_set_t;



typedef struct {
    uint8_t wipower : 1;
    uint8_t scramble : 1;
    uint8_t _44_1k : 1;
    uint8_t _48k : 1;
    uint8_t single_vs : 1;
    uint8_t sbc_encoding : 1;
    uint8_t : 1;
    uint8_t : 1;

    uint8_t sbc_source : 1;
    uint8_t mp3_source : 1;
    uint8_t aac_source : 1;
    uint8_t ldac_source : 1;
    uint8_t aptx_source : 1;
    uint8_t aptx_hd_source : 1;
    uint8_t aptx_adaptive_source : 1;
    uint8_t aptx_twsplus_source : 1;

    uint8_t sbc_sink : 1;
    uint8_t mp3_sink : 1;
    uint8_t aac_sink : 1;
    uint8_t ldac_sink : 1;
    uint8_t aptx_sink : 1;
    uint8_t aptx_hd_sink : 1;
    uint8_t aptx_adaptive_sink : 1;
    uint8_t aptx_twsplus_sink : 1;

    uint8_t dual_sco : 1;
    uint8_t dual_esco : 1;
    uint8_t aptx_voice : 1;
    uint8_t lhdc_source : 1;
    uint8_t qle_hci: 1;
    uint8_t qcm_hci: 1;
    uint8_t aac_source_abr: 1;
    uint8_t aptx_adaptive_source_split_tx : 1;

    uint8_t broadcast_tx_25: 1;
    uint8_t broadcast_tx_39: 1;
    uint8_t broadcast_rx_25: 1;
    uint8_t broadcast_rx_39: 1;
    uint8_t iso_cig_param_calc: 1;
    uint8_t bqr_ext : 1;
    uint8_t : 1;
    uint8_t : 1;

    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;

    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;

    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
    uint8_t : 1;
} bt_soc_addon_features_bitfields_t;

static_assert(sizeof(bt_soc_addon_features_bitfields_t) == 8);

typedef struct {
    uint16_t product_id;
    uint16_t response_version;
    uint8_t valid_bytes;
    union {
        bt_soc_addon_features_bitfields_t as_struct;
        uint8_t as_array[256];
    };
} bt_device_soc_addon_features_t;

#define SOC_ADDON_FEATURES_Fmt "product ID 0x%04x, response ver 0x%x, \n    " \
                                BOOL_Fmt("WiPower")", "BOOL_Fmt("Scrambling Required")", "BOOL_Fmt("44.1 kHz")", "BOOL_Fmt("48 kHz")", "BOOL_Fmt("Single VS Command Support")", "BOOL_Fmt("SBC encoding")", \n    " \
                                BOOL_Fmt("SBC Source")", "BOOL_Fmt("MP3 Source")", "BOOL_Fmt("AAC Source")", "BOOL_Fmt("LDAC Source")", "BOOL_Fmt("aptX Source")", "BOOL_Fmt("aptX HD Source")", "BOOL_Fmt("aptX Adaptive Source")", "BOOL_Fmt("aptX TWS+ source")", \n    " \
                                BOOL_Fmt("SBC Sink")", "BOOL_Fmt("MP3 Sink")", "BOOL_Fmt("AAC Sink")", "BOOL_Fmt("LDAC Sink")", "BOOL_Fmt("aptX Sink")", "BOOL_Fmt("aptX HD Sink")", "BOOL_Fmt("aptX Adaptive Sink")", "BOOL_Fmt("aptX TWS+ Sink")", \n    " \
                                BOOL_Fmt("Dual SCO")", "BOOL_Fmt("Dual eSCO")", "BOOL_Fmt("aptX Adaptive Voice")", "BOOL_Fmt("LHDC Source")", "BOOL_Fmt("QLE HCI")", "BOOL_Fmt("QCM HCI")", "BOOL_Fmt("AAC Source ABR")", "BOOL_Fmt("aptX Adaptive Source Split TX")", \n    " \
                                BOOL_Fmt("Broadcast Audio Tx with EC-2:5")", "BOOL_Fmt("Broadcast Audio Tx with EC-3:9")", "BOOL_Fmt("Broadcast Audio Rx with EC-2:5")", "BOOL_Fmt("Broadcast Audio Rx with EC-3:9")", "BOOL_Fmt("ISO CIG Parameter Calculation")", "BOOL_Fmt("BQR Ext")

#define SOC_ADDON_FEATURES_Arg(x) (x).product_id, (x).response_version, \
                                  BOOL_Arg((x).as_struct.wipower), BOOL_Arg((x).as_struct.scramble), BOOL_Arg((x).as_struct._44_1k), BOOL_Arg((x).as_struct._48k), BOOL_Arg((x).as_struct.single_vs), BOOL_Arg((x).as_struct.sbc_encoding), \
                                  BOOL_Arg((x).as_struct.sbc_source), BOOL_Arg((x).as_struct.mp3_source), BOOL_Arg((x).as_struct.aac_source), BOOL_Arg((x).as_struct.ldac_source), BOOL_Arg((x).as_struct.aptx_source), BOOL_Arg((x).as_struct.aptx_hd_source), BOOL_Arg((x).as_struct.aptx_adaptive_source), BOOL_Arg((x).as_struct.aptx_twsplus_source), \
                                  BOOL_Arg((x).as_struct.sbc_sink), BOOL_Arg((x).as_struct.mp3_sink), BOOL_Arg((x).as_struct.aac_sink), BOOL_Arg((x).as_struct.ldac_sink), BOOL_Arg((x).as_struct.aptx_sink), BOOL_Arg((x).as_struct.aptx_hd_sink), BOOL_Arg((x).as_struct.aptx_adaptive_sink), BOOL_Arg((x).as_struct.aptx_twsplus_sink), \
                                  BOOL_Arg((x).as_struct.dual_sco), BOOL_Arg((x).as_struct.dual_esco), BOOL_Arg((x).as_struct.aptx_voice), BOOL_Arg((x).as_struct.lhdc_source), BOOL_Arg((x).as_struct.qle_hci), BOOL_Arg((x).as_struct.qcm_hci), BOOL_Arg((x).as_struct.aac_source_abr), BOOL_Arg((x).as_struct.aptx_adaptive_source_split_tx), \
                                  BOOL_Arg((x).as_struct.broadcast_tx_25), BOOL_Arg((x).as_struct.broadcast_tx_39), BOOL_Arg((x).as_struct.broadcast_rx_25), BOOL_Arg((x).as_struct.broadcast_rx_39), BOOL_Arg((x).as_struct.iso_cig_param_calc), BOOL_Arg((x).as_struct.bqr_ext)



int hci_read_add_on_features(int dd, bt_device_soc_addon_features_t *soc, int to) {
    uint8_t buf[HCI_MAX_EVENT_SIZE] = {};
    if (hci_send_cmd(dd, OGF_VS, OCF_VS_ADDON, 0, buf) < 0) {
        perror("Error reading add on features");
        return -1;
    }

    ssize_t len = 0;
    if ((len = read(dd, buf, sizeof(buf))) < 0) {
        perror("Read failed");
        return -1;
    }

#ifdef DEBUG

    printf("Add on features (len %zd)", len);
    hexdump(": ", buf, len);

#endif

    uint8_t parameter_length = buf[1];

    uint8_t *stream = read_command_complete_header(&buf[1], NO_OPCODE_CHECKING, 0);

    if (stream && (parameter_length > 8)) {
      STREAM_TO_UINT16(soc->product_id, stream);
      STREAM_TO_UINT16(soc->response_version, stream);

      soc->valid_bytes = parameter_length - 8;
      STREAM_TO_ARRAY(soc->as_array, stream, soc->valid_bytes);
    }

    return 0;
}


bool is_qti_controller(struct hci_version *ver) {
    for (size_t i = 0; i < ARRAY_SIZE(versions); i++) {
        if (ver->manufacturer == versions[i].comp_id &&
            ver->lmp_ver >= versions[i].min_lmp_version &&
            ver->lmp_subver >= versions[i].min_lmp_sub_version) return true;
    }
    return false;
}

static const char *ver_map[] = {
    "1.0b",
    "1.1",
    "1.2",
    "2.0",
    "2.1",
    "3.0",
    "4.0",
    "4.1",
    "4.2",
    "5.0",
    "5.1",
    "5.2",
    "5.3",
    "5.4",
};


int main(int argc, char **argv) {
    int dev_id = hci_devid("hci0");
    bdaddr_t addr;
    struct hci_filter flt;


    if (hci_devba(dev_id, &addr) < 0) {
        perror("hci0 is missing");
        return 1;
    }

    printf("Local address: "BDADDR_Fmt"\n", BDADDR_Arg(addr));

    struct hci_version ver = {};

    int dd = -1;

    if ((dd = hci_open_dev(dev_id)) < 0) {
        fprintf(stderr, "Can't open device hci%d", dev_id);
        perror("");
        return 1;
    }

    /* Setup filter */
    hci_filter_clear(&flt);
    hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
    hci_filter_all_events(&flt);
    if (setsockopt(dd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
        perror("HCI filter setup failed");
        return 0;
    }


    hci_read_local_version(dd, &ver, 1000);

    printf("HCI version %s (0x%x), revision 0x%x\n", ver_map[ver.hci_ver], ver.hci_ver, ver.hci_rev);
    printf("LMP version %s (0x%x), subversion 0x%x\n", ver_map[ver.lmp_ver], ver.lmp_ver, ver.lmp_subver);
    printf("Manufacturer is %s (0x%x)\n", bt_compidtostr(ver.manufacturer), ver.manufacturer);

    printf("QTI vendor commands %s\n", is_qti_controller(&ver) ? "*should* be supported" : "are definitely not supported");

    if (!is_qti_controller(&ver)) {
        printf("Not QTI controller, nothing more to do\n");
        hci_close_dev(dd);
        return 0;
    }

    bt_device_soc_addon_features_t soc = {};

    if (hci_read_add_on_features(dd, &soc, 1000) < 0) {
        return 1;
    }

    if (soc.as_struct.qle_hci == 0) {
        printf("Old device, QLE HCI is not supported, nothing more to do\n");
        hci_close_dev(dd);
        return 0;
    }

    printf("Device SOC features: \n    "SOC_ADDON_FEATURES_Fmt"\n", SOC_ADDON_FEATURES_Arg(soc));

    qlmp_feature_set_t qlmp = {};

    if (hci_read_local_qlmp_features(dd, &qlmp, 1000) < 0) {
        return 1;
    }

    printf("QLMP features: \n    "QLMP_FEATURE_SET_Fmt"\n", QLMP_FEATURE_SET_Arg(qlmp));

    hci_close_dev(dd);
    return 0;
}


int main1() {
    //qlmp_feature_set_t qlmp = {
    //    .bredr_qhs_p2 = 1, .qhs_p3 = 1, .qhs_p4 = 1, .qhs_p5 = 1, .qhs_p6 = 1,
    //    .high_level_ch_msg = 1, .esco_dtx = 1, .tws_esco = 1, .split_acl = 1,
    //    .nonce = 1, .rt_soft_comb = 0, .frozen_clk = 0, .rtp_burst = 0,
    //};

    qlmp_feature_set_t qlmp = {
        .qhs_p5 = 1//, .qhs_p4 = 1, .qhs_p3 = 1, .bredr_qhs_p2 = 1,
    };

    printf("QLMP features: \n    "QLMP_FEATURE_SET_Fmt"\n", QLMP_FEATURE_SET_Arg(qlmp));

    uint8_t buf[16];

    memcpy(buf, &qlmp, 16);

    hexdump("QLMP: ", buf, 16);

    return 0;
}

int main0(int argc, char **argv) {

    union {
        struct {
            uint8_t sbc_source : 1;
            uint8_t mp3_source : 1;
            uint8_t aac_source : 1;
            uint8_t ldac_source : 1;
            uint8_t aptx_source : 1;
            uint8_t aptx_hd_source : 1;
            uint8_t aptx_adaptive_source : 1;
            uint8_t aptx_twsplus_source : 1;
        } as_struct;

        uint8_t as_uint8;
    } vsoc, *soc = &vsoc;

    if (argc < 2) {
        vsoc.as_uint8 = 0x7d;
    } else {
        vsoc.as_uint8 = atoi(argv[1]);
    }

    printf("%d %d\n", soc->as_struct.sbc_source, soc->as_uint8 & 0x01);

    printf(BOOL_Fmt("sbc source")", "BOOL_Fmt("mp3 source")", "BOOL_Fmt("aac source")", "BOOL_Fmt("ldac source")", "BOOL_Fmt("aptx source")", "BOOL_Fmt("aptx hd source")", "BOOL_Fmt("aptx ad source")", ""\n", BOOL_Arg(soc->as_struct.sbc_source), BOOL_Arg(soc->as_struct.mp3_source), BOOL_Arg(soc->as_struct.aac_source), BOOL_Arg(soc->as_struct.ldac_source), BOOL_Arg(soc->as_struct.aptx_source), BOOL_Arg(soc->as_struct.aptx_hd_source), BOOL_Arg(soc->as_struct.aptx_adaptive_source));
    printf(BOOL_Fmt("sbc source")", "BOOL_Fmt("mp3 source")", "BOOL_Fmt("aac source")", "BOOL_Fmt("ldac source")", "BOOL_Fmt("aptx source")", "BOOL_Fmt("aptx hd source")", "BOOL_Fmt("aptx ad source")"\n", BOOL_Arg(0x7d&0x1), BOOL_Arg(0x7d&0x2), BOOL_Arg(0x7d&0x4), BOOL_Arg(0x7d&0x8), BOOL_Arg(0x7d&0x10), BOOL_Arg(0x7d&0x20), BOOL_Arg(0x7d&0x40));

    return 0;
}
