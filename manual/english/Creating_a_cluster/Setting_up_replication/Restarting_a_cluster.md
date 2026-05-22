# Restarting a cluster

When a whole replication cluster is down, one node must be started first so the other nodes know which copy of the cluster to join.

That first-start decision is based on `grastate.dat`, the small replication state file stored in the cluster data directory. The most important fields are:

- `seqno` - the last transaction number known to that node
- `safe_to_bootstrap` - whether that node is marked as safe to start first after a clean shutdown

Example of what `grastate.dat` can look like after a clean shutdown:

```text
# saved replication state
version: 2.1
uuid:    <cluster-uuid>
seqno:   12345
safe_to_bootstrap: 1
```

In this example:

- `seqno: 12345` means this node knows about transactions up to sequence number 12345
- `safe_to_bootstrap: 1` means this node is marked as safe to start first

If the whole cluster was shut down cleanly, start the node that was stopped last. In practice, this is usually the node with:

- the most advanced `seqno`
- `safe_to_bootstrap: 1`

Start that node first with the `--new-cluster` command-line option, or run `manticore_new_cluster` on Linux through systemd. This tells Manticore to start a new copy of the cluster from that node. After that, start the remaining nodes normally so they can rejoin.

If another node is started first without the required clean-shutdown state, startup is refused to protect the cluster from being restored from an older copy. The `--new-cluster-force` option overrides that protection and forces the cluster to start from the chosen node. On Linux, you can use `manticore_new_cluster --force`.

If all nodes crashed or were shut down uncleanly, `grastate.dat` may no longer be trustworthy for normal bootstrap selection. In that case, find the node with the most recent data, usually the one with the largest `seqno`, and start it with `--new-cluster-force`.

<!-- proofread -->
