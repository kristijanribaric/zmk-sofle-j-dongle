/*
 * Copyright (c) 2026 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * Compatibility shim for ESB split peripherals.
 *
 * The stock `nice_view` peripheral widget
 * (app/boards/shields/nice_view/widgets/peripheral_status.c) calls
 * `zmk_split_bt_peripheral_is_connected()` and subscribes to the
 * `zmk_split_peripheral_status_changed` event. Both are normally
 * provided by `app/src/split/bluetooth/peripheral.c`, which is only
 * compiled when `CONFIG_ZMK_SPLIT_BLE=y`.
 *
 * With the ESB split transport the BLE peripheral source is not built,
 * so the widget fails to link. ESB is connectionless from the
 * peripheral's point of view, so the best we can report is "connected"
 * (the peripheral is up and transmitting). This shim provides a
 * constant `true` implementation of the connection predicate.
 *
 * NOTE: the `zmk_split_peripheral_status_changed` event itself is
 * defined unconditionally in `app/src/events/split_peripheral_status_changed.c`,
 * so we do not need to (and must not) re-implement it here.
 */

#include <zephyr/kernel.h>

bool zmk_split_bt_peripheral_is_connected(void) {
    return true;
}

bool zmk_split_bt_peripheral_is_bonded(void) {
    return true;
}
