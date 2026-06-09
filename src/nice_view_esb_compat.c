/*
 * Compatibility shim for nice!view's BLE-specific peripheral status widget
 * when the split transport is Nordic ESB instead of ZMK split BLE.
 */

#include <zephyr/kernel.h>
#include <zmk/split/bluetooth/peripheral.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ESB) && !IS_ENABLED(CONFIG_ZMK_SPLIT_BLE)
bool zmk_split_bt_peripheral_is_connected(void) { return true; }

bool zmk_split_bt_peripheral_is_bonded(void) { return true; }
#endif
