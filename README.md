# nested-data v0.1.0

Nested-data, a library to make creating, querying, mutating, and accessing nested data formats easier.

This project currently only supports JSON, coming soon is YAML.

You can find an example in `examples/example.cpp`, and the include path is `include/...`.

# Coming soon: v0.2.0

This version will add:

1. More generics, such as iterators.
2. An easier API, such as using `operator[]` instead of `nested_data::json::JSON::child()` for accessing both children and nodes.
3. More overloads for `nested_data::json::json`.
