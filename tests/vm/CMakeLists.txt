project(vm-testing)

enable_testing()

set(SRC ${PROJECT_SOURCE_DIR}/../../src)
include_directories(${PROJECT_SOURCE_DIR} ${SRC})

if(DISABLE_TRACING)
    set(TRACESUPPORT "")
else(DISABLE_TRACING)
    set(TRACESUPPORT ${SRC}/XmlTraceLog.cpp ${SRC}/XmlTraceLogTags.cpp)
endif(DISABLE_TRACING)

# I build the vm code here since it needs to be built for both code threading
# models rather than linking against the library from source.
add_library(vm-test-common  STATIC
                basic_test.cpp
                ${SRC}/gr_face.cpp
                ${SRC}/Bidi.cpp
                ${SRC}/CachedFace.cpp
                ${SRC}/Code.cpp
                ${SRC}/CmapCache.cpp
                ${SRC}/Face.cpp
                ${SRC}/FeatureMap.cpp
                ${SRC}/GlyphFace.cpp
                ${SRC}/GlyphFaceCache.cpp
                ${SRC}/NameTable.cpp
                ${SRC}/Pass.cpp
                ${SRC}/Segment.cpp
                ${SRC}/SegCache.cpp
                ${SRC}/SegCacheEntry.cpp
                ${SRC}/SegCacheStore.cpp
                ${SRC}/Silf.cpp
                ${SRC}/Slot.cpp
                ${SRC}/TtfUtil.cpp
                ${TRACESUPPORT})

if  (${CMAKE_COMPILER_IS_GNUCXX})
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer")
endif  (${CMAKE_COMPILER_IS_GNUCXX})

add_executable(vm-test-call     ${SRC}/call_machine.cpp)
target_link_libraries(vm-test-call vm-test-common)

if  (${CMAKE_COMPILER_IS_GNUCXX})
	add_executable(vm-test-direct   ${SRC}/direct_machine.cpp)
	target_link_libraries(vm-test-direct vm-test-common)
endif  (${CMAKE_COMPILER_IS_GNUCXX})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE -DGR2_EXPORTING)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

add_definitions(-DDISABLE_TRACING)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_definitions(-fno-rtti -fno-exceptions)
	if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
		add_definitions(-DNDEBUG -fomit-frame-pointer)
	endif ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
endif  (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

add_test(vm-test-call-threading vm-test-call ${PROJECT_SOURCE_DIR}/../fonts/Padauk.ttf 1)
set_tests_properties(vm-test-call-threading PROPERTIES
        PASS_REGULAR_EXPRESSION "simple program size:    14 bytes.*result of program: 42"
        FAIL_REGULAR_EXPRESSION "program terminated early;stack not empty")

if  (${CMAKE_COMPILER_IS_GNUCXX})
	add_test(vm-test-direct-threading vm-test-direct ${PROJECT_SOURCE_DIR}/../fonts/Padauk.ttf 1)
	set_tests_properties(vm-test-direct-threading PROPERTIES
			PASS_REGULAR_EXPRESSION "simple program size:    14 bytes.*result of program: 42"
			FAIL_REGULAR_EXPRESSION "program terminated early;stack not empty")
endif  (${CMAKE_COMPILER_IS_GNUCXX})
