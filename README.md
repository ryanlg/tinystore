# tinystore

A small key-value store with a write-ahead log, written in C++23. Started during a layoff to
keep my hands busy, and because I'd read plenty about how databases stay durable but had never
built any of it myself.

Still in early stages.

## How it works

Use a `Store` to call `put`. It gives the write a sequence number (an LSN), appends
it to the log file, and then hands it to a backend. The only backend right now is an in-memory
`unordered_map`. A `get` skips the log and asks the backend directly.

The log is one big append-only file called `wal`, created in whatever directory you run from. Each
write becomes a small binary record:

| Field        | Size     | Notes            |
| ------------ | -------- | ---------------- |
| op           | 1 byte   |                  |
| LSN          | 8 bytes  | little-endian    |
| key length   | 2 bytes  | little-endian    |
| value length | 4 bytes  | little-endian    |
| key          | variable | raw bytes        |
| value        | variable | raw bytes        |

So keys max out at 64 KiB and values at 4 GiB. No particular reason behind the limits.

## Some choices I made

**Errors come back as values.** Encoding and decoding return `std::expected`, so a key that's too
long shows up as an error you have to deal with. But log itself should probably throw instead.

**Backends hide behind an interface.** Eventually I'd like a Rust layer on top that wires backends
together from a config file and handles client connections. It'd be pretty fun to write an FFI for
this, so I'm trying to keep the public interface small.

## Building

This project uses Nix to manage the dev environment. It is hooked up to direnv, otherwise run
`nix develop` first.

```sh
cmake -S . -B build -G Ninja
just build
just test
```

There's also `just fmt` and `just lint` for formatting and linting.

It only builds on macOS right now. The log's file syncing is Mac-specific and I haven't
written the Linux version yet.

## What's next

Roughly in order:

- Read the log back on startup
- Add a checksum to each record so it can catch torn writes
- Batching writes
- Checkpoints for WAL
- The Rust frontend and an interactive client, once the core feels solid

See `todo.md` if you want to read the messier version of it.
