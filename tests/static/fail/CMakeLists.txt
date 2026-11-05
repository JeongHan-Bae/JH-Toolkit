project_static_fail(enum_case_rejects_unscoped_enum concepts/enum_case_rejects_unscoped_enum.cpp)
project_static_fail(
    to_range_rejects_nonmovable_temporary
    concepts/to_range_rejects_nonmovable_temporary.cpp
    EXPECT_DIAGNOSTIC "deleted"
)
project_static_fail(
    to_range_rejects_temporary_initializer_list
    concepts/to_range_rejects_temporary_initializer_list.cpp
    EXPECT_DIAGNOSTIC "constraints not satisfied"
)
project_static_fail(
    serializable_rejects_temporary_borrowed_string
    concepts/serializable_rejects_temporary_borrowed_string.cpp
    EXPECT_DIAGNOSTIC "deleted"
)
project_static_fail(
    serializable_rejects_temporary_stream_input
    concepts/serializable_rejects_temporary_stream_input.cpp
    EXPECT_DIAGNOSTIC "deleted"
)
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
    flatten_proxy_rejects_temporary_get_ref
    concepts/flatten_proxy_rejects_temporary_get_ref.cpp
    EXPECT_DIAGNOSTIC "deleted"
)
project_static_fail(
    flatten_proxy_rejects_value_returning_get_ref
    concepts/flatten_proxy_rejects_value_returning_get_ref.cpp
    EXPECT_DIAGNOSTIC "constraints not satisfied"
)

project_static_fail(
    immutable_str_rejects_implicit_c_string
    core/immutable_str_rejects_implicit_c_string.cpp
)
project_static_fail(
    immutable_str_rejects_removed_atomic_str_ptr_alias
    core/immutable_str_rejects_removed_atomic_str_ptr_alias.cpp
    EXPECT_DIAGNOSTIC "atomic_str_ptr"
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

project_static_fail(
    pattern_matcher_rejects_duplicate_rule_ranks
    core/pattern_matcher_rejects_duplicate_rule_ranks.cpp
    EXPECT_DIAGNOSTIC "unique_rule_ranks"
)
project_static_fail(
    pattern_matcher_builder_rejects_duplicate_rule_ranks
    core/pattern_matcher_builder_rejects_duplicate_rule_ranks.cpp
    EXPECT_DIAGNOSTIC "unique_rule_ranks"
)

project_static_fail_finalize()
