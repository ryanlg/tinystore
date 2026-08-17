#Tinystore

A toy KV store with log. My layoff sanity project.

- Core mechanisms are provided as C++23 library.
  - Will think about frontend later, possibly in Rust.
- Support two operations:
  - `put(k, v)`
  - `get(k)`
- In memory only for now, no fault tolerance.
  - Serialization is still support for this in-memory backend so we can write the marshalling before
    getting to the more complicated parts of a file-backed store
  - Serialization of log records should be independent of the underlying storage or transmission format.
    It's a layer above that.
- Interact only programmatically for now through tests.
- Nothing else for now, keeping the initial scope small.
