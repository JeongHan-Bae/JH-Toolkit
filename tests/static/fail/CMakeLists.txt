project_static_fail(enum_case_rejects_unscoped_enum concepts/enum_case_rejects_unscoped_enum.cpp)
project_static_fail(runtime_arr_bool_has_no_data concepts/runtime_arr_bool_has_no_data.cpp)
project_static_fail(
    flatten_proxy_rejects_reference_wrapper
    concepts/flatten_proxy_rejects_reference_wrapper.cpp
    EXPECT_DIAGNOSTIC "deleted"
)
project_static_fail(
    flatten_proxy_rejects_dangling_rvalue_reference
    concepts/flatten_proxy_rejects_dangling_rvalue_reference.cpp
    EXPECT_DIAGNOSTIC "deleted"
)
project_static_fail(
    flatten_proxy_rejects_dangling_const_lvalue_reference
    concepts/flatten_proxy_rejects_dangling_const_lvalue_reference.cpp
    EXPECT_DIAGNOSTIC "deleted"
)

project_static_fail_finalize()
