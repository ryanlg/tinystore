# Store
- [ ] Store should check limits before applying mutations.
- [ ] `put` and `get` should return errors.

# Byte Codec
- [x] Define Record layout, since we have decided that Records are serialised independent to the wire format.
- [x] ~~ByteWriter -> ByteCodec~~
  - [x] We need a reader impl too
- [x] ByteWriter should write to an external vector

# Mutation
- [x] Everything gets pushed through mutations.
  - [ ] "apply" mutations

# Write Ahead Log
- [x] Record should seralize directly to WAL's ByteWriter
- [x] Record should be split into each operation
- [x] WAL should use codec but customize it
- [ ] WAL and serialization should raise exceptions return than returning them.
- LSN
  - [ ] Make atomic
- Batch
  - [x] Serialization
  - [ ] CRC
- [ ] Replay
  - ??
