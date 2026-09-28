project_static_fail(
    duplicate_step_binding
    duplicate_step_binding.cpp
    "StepDefinition cannot bind the same expression more than once"
)
project_static_fail(
    invalid_step_expression
    invalid_step_expression.cpp
    "Invalid Cucumber step expression."
)
project_static_fail(
    invalid_callable_signature
    invalid_callable_signature.cpp
    "Step callable must be a void member or static function"
)
project_static_fail(
    mutable_data_table_binding
    mutable_data_table_binding.cpp
    "Step callable must be a void member or static function"
)

project_static_fail_finalize()
