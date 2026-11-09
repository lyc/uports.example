# uports.example

This workspace demonstrates **uports**, a FreeBSD ports-inspired build and
package system with project group support. Recipes describe software; groups
describe the environments in which that software is built and used.

## uports framework overview

This section describes reusable **framework behavior**. The project-specific
groups, recipes and application commands are introduced in the next section.

| Layer | Responsibility |
|---|---|
| Recipes | Describe sources, versions, dependencies, build/install rules and package manifests. |
| Flavors | Define named variations of one recipe, such as another version or build configuration. |
| Groups | Define package membership, build environments, installation settings and provider policy. |
| Dependency resolver | Select compatible system or uports providers, follow dependencies, detect cycles and order execution. |
| Lifecycle and state | Run package phases and track progress and relevant configuration changes. |
| Package manager | Create, install and remove packages, track installed files and detect ownership conflicts. |

A **recipe, group and Flavor** identify a build instance with separate work and
state paths. Whether instances can be installed together depends on their
prefixes, package identities and installed file paths.

Recipes specify what they need through `BUILD_DEPENDS`, `LIB_DEPENDS` and
`RUN_DEPENDS`, optionally selecting an exact dependency Flavor. Provider policy
decides which system tools or group environments may supply those requirements.
Dependencies are prepared before their consumers, with valid installed packages
reused when possible.

The lifecycle is:

```text
fetch -> extract -> patch -> configure -> build -> stage -> package -> install
```

Staging collects package contents before installation. `DESTDIR` and `PREFIX`
control the installation layout. Development installs prepare sources for review
by default, even when a compatible package skips compilation;
`FORCE_DEVELOPMENT_INSTALL=no` permits reuse without source preparation.

Lifecycle commands use `[group:]port[@flavor].phase`. Group membership defines
aggregate operations, while an explicit command may select any discovered recipe
in a declared group. An omitted group resolves through the default group or a
unique configured group. An omitted Flavor uses its scoped group setting or
the recipe's declared default.

`info.ports` displays versions, licenses and group/Flavor lifecycle status. It
does not resolve dependencies or verify installed-file health. Dependency plans
are inspected separately.

The framework supplies reusable defaults in `make/uports/Tools/group.mk`.
Projects define their memberships, environments and provider policy in
`feeds/group.mk`, then integrate the framework through their parent `Makefile`.
The [framework README](make/uports/README.md) provides more detail and branch
history.

## This example project

The following layout, group names and commands are **uports.example choices**;
other projects can configure different development environments.

### Project layout

| Location | Purpose |
|---|---|
| `make/uports/` | uports framework and built-in recipes. |
| `feeds/group.mk` | This project's group memberships, environments, Flavor defaults and provider policy. |
| `feeds/<category>/<port>/` | Project recipes and overrides of built-in recipes. |
| `Makefile` | Framework integration and application/module selection. |
| `src/`, `libs/`, `tests/` | Application, shared libraries and example tests. |
| `local/` | Default development installation destination. |

The current groups are `compiler`, `bs`, `host`, `textproc` and `toolchain`.
They support language development, bootstrap tools, native build tools, text
processing libraries and cross-toolchain producers respectively. Cross-toolchain
producer executables run on the host while generating code for another target.

### Package commands

Examples using this project's configured groups and recipes:

```sh
make info.ports
make pkg-config.install
make textproc:json-c.install
make compiler:gforth@libffcall.build
make toolchain:gcc@mit2018.build
```

Build/install the complete historical JOS toolchain, or additionally export a
relocatable Linux distribution:

```sh
make i386-jos-elf
make EXPORT=yes i386-jos-elf
```

Export installs host:patchelf and prepares the pinned MIT 2018 lab1 validation
checkout under local/src automatically. JOS_SOURCE can select an existing clean
checkout instead. See the cross-toolchain report for prerequisites, private
runtime libraries, validation and output layout. Ordinary installation does not
build patchelf or fetch the lab checkout.

New exports include a standalone shell validator and the pinned lab source:

```sh
sh /path/to/export/validate.sh
sh /path/to/export/validate.sh compiler
```

The scripts use the exported folder plus standard Linux host utilities and
write results to a separate temporary directory. They need no workspace,
Python, Git or external lab checkout.
The shipped JOS source also retains independent Git history and the MIT origin
for course use with host Git on a writable copy.
Validators print the exercised shell commands, C probe and GDB commands as they
run, while preserving result logs and exit-status checks.

Inspect dependencies separately from the package listing:

```sh
make DEPENDENCY_REQUEST=toolchain:gcc@mit2018.build dependency-execution-plan
```

Filter the listing to a group or package:

```sh
make INFO_PORTS_GROUP=toolchain info.ports
make INFO_PORTS_PORT=gforth info.ports
```

Clean one Flavor's work tree:

```sh
make compiler:gforth@libffcall.distclean
```

### Application development

Prepare the required development packages, then build the application or tests:

```sh
make
./run.sh ts

make test
./run.sh t-tree
```

Use `./run.sh -d t-tree` to run under Valgrind when available.

## Further reading

- [uports framework overview](make/uports/README.md)
- [Cross-toolchain design](feeds/lang/11-CROSS-TOOLCHAIN.md)
