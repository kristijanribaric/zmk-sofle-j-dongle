/*
 * Copyright (c) 2026 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * Software VCOM toggler for the Sharp LS0XX memory-LCD on nice!view
 * shields, working around a Zephyr 4.1 limitation.
 *
 * Background: Sharp memory LCDs (LS013B7DH05, used by nice!view) need
 * the VCOM polarity inverted roughly once per second. There are two
 * supported ways to do that with Zephyr's `sharp,ls0xx` driver:
 *   1. Wire EXTCOMIN to a GPIO and let the driver toggle it (the panel
 *      itself drives VCOM from EXTCOMIN).
 *   2. Set `serial-vcom-inversion` on the DT node and let the driver
 *      send "maintain" commands with the VCOM bit toggled.
 * nice!view uses path #2, but `serial-vcom-inversion` was only added
 * to the upstream LS0XX driver after Zephyr 4.4. The Zephyr 4.1 fork
 * shipped with the badjeff/NCS tree silently ignores that DT property,
 * so VCOM never inverts and the LCD develops a DC bias — visible as
 * the screen rapidly "flickering"/inverting.
 *
 * Until the LS0XX driver in the toolchain catches up, this file
 * implements path #2 from user space: a low-priority thread that
 * issues the bare "maintain + VCOM toggle" SPI command (no pixel data)
 * on the same SPI bus the driver uses. Zephyr's SPI subsystem
 * serialises bus access internally, so it is safe to share with the
 * driver's pixel writes.
 *
 * The command per the LS013B7DH05 datasheet is a two-byte transfer:
 *   byte 0: 0x00 (maintain), with bit 1 = VCOM polarity
 *   byte 1: 0x00 (no line address / trailer)
 * Sending 0x02,0x00 then 0x00,0x00 each second flips VCOM at 0.5 Hz,
 * which is well within the panel's recommended 0.5–1 Hz range.
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/spi.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ls0xx_vcom, CONFIG_ZMK_LOG_LEVEL);

#define LS0XX_NODE DT_CHOSEN(zephyr_display)

BUILD_ASSERT(DT_NODE_HAS_COMPAT(LS0XX_NODE, sharp_ls0xx),
             "ls0xx_vcom_toggle expects zephyr,display to be a sharp,ls0xx node");

static const struct spi_dt_spec ls0xx_spi = SPI_DT_SPEC_GET(
    LS0XX_NODE,
    SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
    0);

#define LS0XX_BIT_VCOM 0x02

static void ls0xx_vcom_thread(void *a, void *b, void *c) {
    ARG_UNUSED(a);
    ARG_UNUSED(b);
    ARG_UNUSED(c);

    /* Give the LS0XX driver a moment to do its own init / SPI setup
     * before we start sharing the bus. */
    k_msleep(2000);

    if (!spi_is_ready_dt(&ls0xx_spi)) {
        LOG_ERR("LS0XX SPI bus not ready, VCOM toggle disabled");
        return;
    }

    bool vcom_state = false;
    while (true) {
        uint8_t cmd[2] = { vcom_state ? LS0XX_BIT_VCOM : 0x00, 0x00 };
        vcom_state = !vcom_state;

        struct spi_buf tx_buf = { .buf = cmd, .len = sizeof(cmd) };
        struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };

        int err = spi_write_dt(&ls0xx_spi, &tx_set);
        if (err) {
            LOG_WRN("VCOM toggle SPI write failed: %d", err);
        }

        k_msleep(CONFIG_ZMK_SOFLE_LS0XX_VCOM_INTERVAL_MS);
    }
}

K_THREAD_DEFINE(ls0xx_vcom_tid,
                CONFIG_ZMK_SOFLE_LS0XX_VCOM_STACK_SIZE,
                ls0xx_vcom_thread, NULL, NULL, NULL,
                K_LOWEST_APPLICATION_THREAD_PRIO, 0, 0);
