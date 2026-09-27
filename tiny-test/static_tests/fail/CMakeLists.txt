project_static_fail(duplicate_test_definition duplicate_test_definition.cpp "redefinition")
project_static_fail(duplicate_session_definition duplicate_session_definition.cpp "redefinition")
project_static_fail(duplicate_test_name_in_session duplicate_test_name_in_session.cpp "TinyTest session and test names must be unique")
project_static_fail(duplicate_names_in_suite duplicate_names_in_suite.cpp "TinyTest suite, session, and test names must be unique")

add_executable(tiny_test_static_fail_duplicate_test_across_tus
    duplicate_registration_main.cpp
    duplicate_test_tu_one.cpp
    duplicate_test_tu_two.cpp
)
target_link_libraries(tiny_test_static_fail_duplicate_test_across_tus PRIVATE jh::test::jh-tiny-test)
target_compile_features(tiny_test_static_fail_duplicate_test_across_tus PRIVATE cxx_std_20)
add_test(NAME tiny_test_static_fail_duplicate_test_across_tus
    COMMAND tiny_test_static_fail_duplicate_test_across_tus test)

add_executable(tiny_test_static_fail_duplicate_session_across_tus
    duplicate_registration_main.cpp
    same_session_tu_one.cpp
    same_session_tu_two.cpp
)
target_link_libraries(tiny_test_static_fail_duplicate_session_across_tus PRIVATE jh::test::jh-tiny-test)
target_compile_features(tiny_test_static_fail_duplicate_session_across_tus PRIVATE cxx_std_20)
add_test(NAME tiny_test_static_fail_duplicate_session_across_tus
    COMMAND tiny_test_static_fail_duplicate_session_across_tus session)
