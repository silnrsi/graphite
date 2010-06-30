project(vm-testing)

enable_testing()

set(SRC ${PROJECT_SOURCE_DIR}/../../src)
include_directories(${PROJECT_SOURCE_DIR} ${SRC})

# I build the vm code here since it needs to be built for both code threading
# models rather than linking against the library from source.
add_library(vm-test-common  STATIC
                basic_test.cpp
                ${SRC}/Code.cpp
                ${SRC}/FeaturesHandle.cpp
                ${SRC}/GlyphFace.cpp
                ${SRC}/LoadedFace.cpp
                ${SRC}/Pass.cpp
                ${SRC}/Silf.cpp
                ${SRC}/Slot.cpp
                ${SRC}/TtfUtil.cpp)

add_executable(vm-test-call     ${SRC}/call_machine.cpp)
target_link_libraries(vm-test-call vm-test-common)
add_executable(vm-test-direct   ${SRC}/direct_machine.cpp)
target_link_libraries(vm-test-direct vm-test-common)

add_definitions("-fno-rtti -fno-exceptions -DNDEBUG -DDISABLE_TRACING -UENABLE_DEEP_TRACING")


add_test(vm-test-call-threading vm-test-call 1)
set_tests_properties(vm-test-call-threading PROPERTIES
        PASS_REGULAR_EXPRESSION "simple program size:    14 bytes.*result of program: 42"
        FAIL_REGULAR_EXPRESSION "program terminated early;stack not empty")
add_test(vm-test-direct-threading vm-test-direct 1)
set_tests_properties(vm-test-direct-threading PROPERTIES
        PASS_REGULAR_EXPRESSION "simple program size:    14 bytes.*result of program: 42"
        FAIL_REGULAR_EXPRESSION "program terminated early;stack not empty")

