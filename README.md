#Tinystore

A toy KV store with log. My layoff sanity project.

- Core mechanisms are provided as C++23 library.
  - Will think about frontend later, possibly in Rust.
- Support two operations:
  - `put(k, v)`
  - `get(k)`
- In memory only for now\*, no fault tolerance.
  - Serialization is still support for this in-memory backend so we can write the marshalling before
    getting to the more complicated parts of a file-backed store
  - Serialization of log records should be independent of the underlying storage or transmission format.
    It's a layer above that.
  - \*However, WAL should write to disk already, otherwise it has no use.
    - It can just be a giant file for now. We can think about segmentation later.
    - Start with replaying _every write_. Checkpoints are for later.
- Interact only programmatically for now through tests.
- Nothing else for now, keeping the initial scope small.

## Thinking Ahead
- Frontend would be done in Rust, with C++ writing the actual code.
  - No particular reason, just because I can and it's fun.
  - The cut off would be the "backends". Rust would be composing these backends together according to a config file.
  - So the backends are our public API, everything else is encapsulated.
  - Rust would be handling client connections too, and handing off requests to C++ backends.
- Interactive client in Rust.
