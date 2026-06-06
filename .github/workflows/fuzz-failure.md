---
title: Scheduled fuzz run {{ env.FUZZ_TARGET }} unhealthy
labels: fuzz-failure
---
The scheduled fuzz run for `{{ env.FUZZ_TARGET }}` exited non-zero.

This usually means libFuzzer found a crash, leak, OOM, or timeout. The
action run is {{ env.FUZZ_ACTION_RUN }} -- download the
`{{ env.FUZZ_TARGET }}-crashes-*` artifact from that run for the
reproducer file(s).

To reproduce locally:

```
cd fuzz
make {{ env.FUZZ_TARGET }}
./{{ env.FUZZ_TARGET }} <reproducer-file>
```

This issue will be reused for subsequent failures while it is open.
Close it once the underlying problem is fixed.
