# SPDX-License-Identifier: MIT OR MPL-2.0 OR LGPL-2.1-or-later OR GPL-2.0-or-later
# Copyright 2010, SIL International, All rights reserved.
project(nametabletest)
include_directories(${graphite2_core_SOURCE_DIR})

add_executable(nametabletest nametabletest.cpp)
target_link_libraries(nametabletest graphite2-base)

add_test(NAME nametabletest COMMAND $<TARGET_FILE:nametabletest>)
set_tests_properties(nametabletest PROPERTIES TIMEOUT 3)
