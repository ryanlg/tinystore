# Byte Codec
- [x] Define Record layout, since we have decided that Records are serialised independent to the wire format.
- [x] ByteWriter -> ByteCodec.
  - [x] We need a reader impl too.

# Write Ahead Log
- [ ] Record should seralize directly to WAL's ByteWriter
- [ ] Record should be split into each operation
- LSN
  - [ ] Make atomic
- Batch
  - [x] Serialization
  - [ ] CRC
- [ ] Replay
  - ...
