# SPDX-License-Identifier: MIT OR MPL-2.0 OR LGPL-2.1-or-later OR GPL-2.0-or-later
# Copyright 2010, SIL International, All rights reserved.
project(grlisttest)

include_directories(${graphite2_core_SOURCE_DIR})

set(S ${graphite2_core_SOURCE_DIR})

add_executable(grlisttest grlisttest.cpp)
add_test(NAME grlist COMMAND $<TARGET_FILE:grlisttest>)

# add_executable(intervalsettest intervalsettest.cpp)
# target_link_libraries(intervalsettest graphite2 graphite2-base)
# add_test(NAME intervalset COMMAND $<TARGET_FILE:intervalsettest>)
