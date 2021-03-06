This repository contains two distinct pieces of software.

QSRV
----

A PV Access (protocol) server to be included in an EPICS IOC.

```
myioc_DBD += qsrv.dbd
myioc_LIBS += qsrv
```

For convenience an executable `softIocPVA' is also built which is equivalent to the
'softIoc' executable from EPICS Base with the addition of QSRV.

p2p
---

A PV Access gateway (aka proxy).
The 'p2p' executable.

Dependencies
------------

- [epics-base](http://www.aps.anl.gov/epics/) >= 3.15.3
- [pvDataCPP](http://epics-pvdata.sourceforge.net/)
- [pvAccessCPP](http://epics-pvdata.sourceforge.net/)

Building
--------

To build all dependencies from source:

```sh
git clone https://github.com/epics-base/epics-base.git
git clone https://github.com/epics-base/pvDataCPP.git
git clone https://github.com/epics-base/pvAccessCPP.git
git clone https://github.com/mdavidsaver/pva2pva.git

make -C epics-base
make -C pvDataCPP
make -C pvAccessCPP
make -C pva2pva
```

Running QSRV
------------

Any IOC which includes QSRV will automatically start a PV Access server
which exposes all channels (aka. "recordname.FLD") in the same manner
as the built-in Channel Access (protocol) server.

QSRV Group Definitions
----------------------

The following .db file snippet defines a group PV "grp:name"
to have two sub-structures "A" and "B".
Each sub-structure encodes the value and meta data one PV.
eg. "recname.VAL" is stored in "grp:name.A"
and "other.VAL" is "grp:name.B".

```
record(longin, "recname") {
    info(Q:group, {
        "grp:name":{
            "A":{
                +channel:"VAL"
            }
        }
    })
}
record(longin, "other") {
    info(Q:group, {
        "grp:name":{
            "B":{
                +channel:"VAL"
            }
        }
    })
}
```

A full list of `info(Q:group` options.

```
record(...) {
    info(Q:group, {
        "<group_name>":{
            +id:"some/NT:1.0",  // top level ID
            +meta:"FLD",        // map top level alarm/timeStamp
            +atomic:true,       // whether monitors default to multi-locking atomicity
            "<field.name>":{
                +type:"scalar", // controls how map VAL mapped onto <field.name>
                +channel:"VAL",
                +id:"some/NT:1.0",
                +trigger:"*",   // "*" or comma seperated list of <field.name>s
                +putorder:0,    // set for fields where put is allowed, processing done in increasing order
            }
        }
    })
}
```

Running p2p
-----------

pva2pva gateway is intended for use on a computer with at least two ethernet interfaces.
At present each pva2pva process can act as a uni-directional proxy,
presenting a pvAccess server on one interface,
and a client on other(s).

The file [example.cmd](example.cmd) provides a starting point.
Adjust *EPICS_PVAS_INTF_ADDR_LIST* and *EPICS_PVA_ADDR_LIST*
according to the host computer's network configuration.

At present there are no safe guard against creating loops
where a gateway client side connects to its own server side.
To avoid this ensure that the address list does not contain
the interface used for the server (either directly, or included in a broadcast domain).
*EPICS_PVA_AUTO_ADDR_LIST* __must__ remain set to *NO*.

```
cd pva2pva
./bin/linux-x86_64/pva2pva example.cmd
```
