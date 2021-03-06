/**
@mainpage pva2pva Home of QSRV and pvAccess 2 pvAccess gateway

- [Download](https://sourceforge.net/projects/epics-pvdata/files/)

@section qsrv QSRV

QSRV is a network server using the PVAccess protocol which
runs inside an EPICS IOC process and allows clients
to make requests to access the Process Variables (PVs)
within.

@subsection qsrv_build Building

To build the latest from version control

@code
git clone --recursive --branch core/master https://github.com/epics-base/epics-base.git
cd epics-base
make
@endcode

@subsection qsrv_quick Quick Start

The pva2pva module builds an executable 'softIocPVA' which function like the 'softIoc' executable
built by EPICS Base, with QSRV included as well.

@code
cd modules/pva2pva
cat <<EOF > p2pexample.db
record(calc, "p2p:example:counter") {
    field(INPA, "p2p:example:counter")
    field(CALC, "A+1")
    field(SCAN, "1 second")
}
EOF
./bin/linux-x86_64/softIocPVA -d p2pexample.db
@endcode

Then in another shell run:

@code
cd modules/pvAccess
./bin/linux-x86_64/pvget p2p:example:counter
@endcode

@subsection qsrv_include Adding QSRV to your IOC

QSRV is added to an IOC just like any other EPICS support module.
In the Makefile which produces an IOC executable (eg. "myiocname")
add:

@code
PROD_HOST += myiocname
# include QSRV
myiocname_DBD += base.dbd
myiocname_DBD += PVAServerRegister.dbd
myiocname_DBD += qsrv.dbd
myiocname_LIBS += qsrv
myiocname_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)
myiocname_LIBS += $(EPICS_BASE_IOC_LIBS)
@endcode

Now run your IOC and QSRV starts automatically.

@subsubsection qsrv_quick_sts Status monitoring

In a running IOC w/ QSRV, run the "pvasr" in the IOC shell and verify
that "QSRV" is among the "PROVIDER_NAMES".

@code
epics> pvasr
VERSION : pvAccess Server v6.0.0-SNAPSHOT
PROVIDER_NAMES : QSRV,
BEACON_ADDR_LIST :
AUTO_BEACON_ADDR_LIST : 1
BEACON_PERIOD : 15
BROADCAST_PORT : 5076
SERVER_PORT : 5075
RCV_BUFFER_SIZE : 16384
IGNORE_ADDR_LIST:
INTF_ADDR_LIST : 0.0.0.0
@endcode

*/
