# SPDX-License-Identifier: MIT OR MPL-2.0 OR LGPL-2.1-or-later OR GPL-2.0-or-later
# Copyright 2011, SIL International, All rights reserved.
project(endian)

include_directories(${graphite2_core_SOURCE_DIR})

add_executable(endiantest endiantest.cpp)
add_test(NAME endian COMMAND $<TARGET_FILE:endiantest> 10)
