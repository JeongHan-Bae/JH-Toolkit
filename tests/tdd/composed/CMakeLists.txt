function(add_tiny_composed_test target source ctest_name toolkit_target)
    add_executable(${target} "${source}")
    target_link_libraries(${target} PRIVATE ${toolkit_target} jh::test::jh-tiny-test)
    target_compile_features(${target} PRIVATE cxx_std_20)
    add_test(NAME "${ctest_name}" COMMAND ${target})
endfunction()

add_tiny_composed_test(tdd_composed_enum_case test_enum_case.cpp tdd.composed.enum_case jh-toolkit-static)
add_tiny_composed_test(tdd_composed_huffman test_huffman.cpp tdd.composed.huffman jh-toolkit)
add_tiny_composed_test(tdd_composed_immutable_str test_immutable_str.cpp tdd.composed.immutable_str jh-toolkit-static)
add_tiny_composed_test(tdd_composed_pod_sys test_pod_sys.cpp tdd.composed.pod_sys jh-toolkit)
add_tiny_composed_test(tdd_composed_proc_cond test_proc_cond.cpp tdd.composed.proc_cond jh-toolkit)
add_tiny_composed_test(tdd_composed_process_lock test_process_lock.cpp tdd.composed.process_lock jh-toolkit)
add_tiny_composed_test(tdd_composed_ranges test_ranges.cpp tdd.composed.ranges jh-toolkit)
add_tiny_composed_test(tdd_composed_sequence_to_range test_sequence_to_range.cpp tdd.composed.sequence_to_range jh-toolkit)
add_tiny_composed_test(tdd_composed_slot test_slot.cpp tdd.composed.slot jh-toolkit)

add_executable(tdd_ipc_condition_scheduler support/process_condition_scheduler.cpp)
add_executable(tdd_ipc_condition_sender support/process_condition_sender.cpp)
add_executable(tdd_ipc_condition_target support/process_condition_target.cpp)
add_executable(tdd_ipc_mutex_worker support/process_mutex_worker.cpp)
add_executable(tdd_ipc_process_exit_worker support/process_exit_worker.cpp)
add_executable(tdd_ipc_abnormal_worker support/process_abnormal_worker.cpp)
foreach(_worker IN ITEMS
        tdd_ipc_condition_scheduler
        tdd_ipc_condition_sender
        tdd_ipc_condition_target
        tdd_ipc_mutex_worker
        tdd_ipc_process_exit_worker
        tdd_ipc_abnormal_worker)
    target_link_libraries(${_worker} PRIVATE jh-toolkit)
    target_compile_features(${_worker} PRIVATE cxx_std_20)
endforeach()

add_dependencies(tdd_composed_proc_cond
    tdd_ipc_condition_scheduler
    tdd_ipc_condition_sender
    tdd_ipc_condition_target)
add_dependencies(tdd_composed_process_lock tdd_ipc_mutex_worker)
add_dependencies(tdd_composed_process_lock tdd_ipc_process_exit_worker tdd_ipc_abnormal_worker)
set_tests_properties(tdd.composed.proc_cond tdd.composed.process_lock PROPERTIES
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:tdd_composed_process_lock>")
