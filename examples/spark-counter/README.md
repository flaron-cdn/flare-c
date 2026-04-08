# spark-counter

Per-edge hit counter using Spark (the per-site local K/V store with TTL).
Each request increments `hits`, persists it for 24 hours, and returns
`hits: NNN`.

**Required capability**: `writes_spark_kv = true`.

Build:

```sh
make examples
```
