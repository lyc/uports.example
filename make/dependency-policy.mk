# Native compiler builds may use validated host libraries and tools, with uports
# packages retained as the deterministic fallback when discovery is unavailable.
DEPENDENCY_PROVIDER_MODE = host

# Host packages may use native system providers or packages from host/bs.
# Never pull compiler or other project groups into host dependency builds.
DEPENDENCY_PROVIDER_UPORTS_GROUPS.host = host bs
