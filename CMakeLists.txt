# Copyright (c) 2026 The ZMK Contributors
# SPDX-License-Identifier: MIT
#
# This user-config repo is treated as a Zephyr module so we can compile a
# small compatibility shim that provides BLE-peripheral symbols that the
# stock nice_view widget links against. With the ESB split transport the
# BLE peripheral source is not built, so these symbols are unresolved
# unless we provide them ourselves.

if(CONFIG_ZMK_SOFLE_ESB_BLE_COMPAT OR CONFIG_ZMK_SOFLE_LS0XX)
    zephyr_library()
endif()

if(CONFIG_ZMK_SOFLE_ESB_BLE_COMPAT)
    zephyr_library_sources(src/esb_ble_compat.c)
endif()

if(CONFIG_ZMK_SOFLE_LS0XX)
    zephyr_library_sources(drivers/display/ls0xx.c)
endif()
