# uports.example
This repository used to demonstrate how to use uports

Do below once only
----------------
```
uports.example> mkdir /opt/distfiles/ports
uports.example> . tests/env.setup
```
NOTE: skip this step if you don't need to cache any uports package

Build uports package:
----------------
```
uports.example> make info.ports
uports.example> make pkg-config.install
uports.example> make json-c.install
```
Build test stuff:
----------------
```
uports.example> make test
uports.example> ls -al objd
uports.example> ./run.sh -d t-tree
uports.example> ./run.sh -d t-json
```
NOTE: use `./run.sh t-tree` instead if you don't have **valgrind** installed

Build application:
----------------
```
uports.example> make
uports.example> ./run.sh -d ts
```
