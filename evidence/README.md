# Canonical experiment evidence

This directory is reserved for the smallest self-contained evidence packages
that support recorded scientific decisions. Each package is placed at:

```text
evidence/<stage>/<contract>/<campaign-id>/
```

The package contains its retained manifests, machine-readable results,
inventories, focused logs, reports, figures, limitations, and a
`retention-manifest.json`. The retention manifest records the original source
path and hash of every retained file, excluding itself.

Build trees, object files, caches, and reproducible binaries are scratch
material and are not retained here. A missing historical source remains marked
as unavailable rather than reconstructed from a later checkout.
