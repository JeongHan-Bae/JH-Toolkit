add_executable(jh_toolkit_bdd_runner
    runners/roundtrip_runner.cpp
)
target_include_directories(jh_toolkit_bdd_runner PRIVATE steps)
target_link_libraries(jh_toolkit_bdd_runner
    PRIVATE jh-toolkit jh_test_cucumber
)
target_compile_features(jh_toolkit_bdd_runner PRIVATE cxx_std_20)
target_compile_definitions(jh_toolkit_bdd_runner PRIVATE
    "JH_TOOLKIT_BDD_FEATURE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/features\"")

add_test(NAME bdd_behavioral_flows COMMAND jh_toolkit_bdd_runner)
