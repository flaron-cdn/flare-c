# plasma-counter

Global cross-edge counter using Plasma (CRDT-replicated K/V). Each request
increments `global_hits` and returns the new total. Plasma replicates the
value to every other edge in the fleet via gossip.

**Required capability**: `writes_plasma_kv = true`.
