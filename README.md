# Tinystore

A toy KV store with log. My layoff sanity project.

- Core mechanisms are provided as C++23 library.
  - Will think about frontend later, possibly in Rust.
- Support two operations:
  - `put(k, v)`
  - `get(k)`
- Writes to a backing log - start with memory for now, no fault tolerance.
- Interact only programmatically for now through tests.
- Nothing else for now, keeping the initial scope small.
