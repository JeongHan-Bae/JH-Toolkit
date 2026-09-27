add_executable(jh_tiny_test_tdd main.cpp)
target_link_libraries(jh_tiny_test_tdd PRIVATE jh::test::jh-tiny-test)
target_compile_features(jh_tiny_test_tdd PRIVATE cxx_std_20)

add_test(NAME tiny_test_tdd COMMAND jh_tiny_test_tdd)
