add_executable(jh_simple_cucumber_tdd
    main.cpp
    pass/steps.hpp
    fail/steps.hpp
)
target_link_libraries(jh_simple_cucumber_tdd
    PRIVATE jh::test::jh-simple-cucumber
)
target_compile_features(jh_simple_cucumber_tdd PRIVATE cxx_std_20)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(jh_simple_cucumber_tdd PRIVATE -fno-rtti)
endif()
target_compile_definitions(jh_simple_cucumber_tdd PRIVATE
    "JH_SIMPLE_CUCUMBER_TDD_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

add_test(NAME simple_cucumber_tdd COMMAND jh_simple_cucumber_tdd)
