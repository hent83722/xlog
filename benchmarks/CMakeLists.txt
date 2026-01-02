file(GLOB BENCH_SOURCES "*.cpp")

foreach(bench ${BENCH_SOURCES})
    get_filename_component(BENCH_NAME ${bench} NAME_WE)
    add_executable(${BENCH_NAME} ${bench})
    target_link_libraries(${BENCH_NAME} PRIVATE Zyrnix fmt::fmt Threads::Threads)
endforeach()
