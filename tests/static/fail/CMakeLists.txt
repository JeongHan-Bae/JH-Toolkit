project_static_fail(enum_case_rejects_unscoped_enum concepts/enum_case_rejects_unscoped_enum.cpp)
project_static_fail(
    runtime_arr_bool_has_no_data
    concepts/runtime_arr_bool_has_no_data.cpp
    EXPECT_DIAGNOSTIC "deleted"
)
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

project_static_fail(
    immutable_str_rejects_implicit_c_string
    core/immutable_str_rejects_implicit_c_string.cpp
)
project_static_fail(
    immutable_str_rejects_single_argument_std_string
    core/immutable_str_rejects_single_argument_std_string.cpp
    EXPECT_DIAGNOSTIC "deleted"
)
project_static_fail(
    immutable_str_rejects_unlocked_string_view
    core/immutable_str_rejects_unlocked_string_view.cpp
    EXPECT_DIAGNOSTIC "deleted"
)
project_static_fail(
    runtime_arr_rejects_copy_construction
    core/runtime_arr_rejects_copy_construction.cpp
    EXPECT_DIAGNOSTIC "deleted"
)
project_static_fail(
    ordered_map_rejects_unordered_key
    core/ordered_map_rejects_unordered_key.cpp
    EXPECT_DIAGNOSTIC "constraints not satisfied"
)
project_static_fail(
    flat_multimap_rejects_unordered_key
    core/flat_multimap_rejects_unordered_key.cpp
    EXPECT_DIAGNOSTIC "constraints not satisfied"
)

project_static_fail_finalize()
