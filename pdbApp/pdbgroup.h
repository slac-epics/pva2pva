#ifndef PDBGROUP_H
#define PDBGROUP_H

#include <istream>
#include <map>
#include <limits>

#include <dbAccess.h>

#include <dbEvent.h>
#include <dbLock.h>

#include <pv/pvAccess.h>

#include "helper.h"
#include "pvahelper.h"
#include "pvif.h"
#include "pdb.h"

#include <shareLib.h>

struct epicsShareClass GroupConfig
{
    struct epicsShareClass Field {
        std::string type, channel, trigger, id;
        int putorder;

        Field() :putorder(std::numeric_limits<int>::min()) {}

        void swap(Field& o) {
            std::swap(type, o.type);
            std::swap(channel, o.channel);
            std::swap(trigger, o.trigger);
            std::swap(putorder, o.putorder);
            std::swap(id, o.id);
        }
    };

    struct epicsShareClass Group {
        typedef std::map<std::string, Field> fields_t;
        fields_t fields;
        bool atomic, atomic_set;
        std::string id;

        Group() :atomic(true), atomic_set(false) {}

        void swap(Group& o) {
            std::swap(fields, o.fields);
            std::swap(atomic, o.atomic);
            std::swap(atomic_set, o.atomic_set);
            std::swap(id, o.id);
        }
    };

    typedef std::map<std::string, Group> groups_t;
    groups_t groups;
    std::string warning;

    void swap(GroupConfig& o) {
        std::swap(groups, o.groups);
        std::swap(warning, o.warning);
    }

    static void parse(const char *txt,
                      GroupConfig& result);
};

struct PDBGroupMonitor;

void pdb_group_event(void *user_arg, struct dbChannel *chan,
                     int eventsRemaining, struct db_field_log *pfl);

struct epicsShareClass PDBGroupPV : public PDBPV
{
    POINTER_DEFINITIONS(PDBGroupPV);
    weak_pointer weakself;
    inline shared_pointer shared_from_this() { return shared_pointer(weakself); }

    epicsMutex lock;

    bool pgatomic, monatomic;

    // get/put/monitor
    std::string name;

    struct Info {
        DBCH chan;
        std::tr1::shared_ptr<PVIFBuilder> builder;
        FieldName attachment;
        typedef std::vector<size_t> triggers_t;
        triggers_t triggers; // index in PDBGroupPV::members
        DBManyLock locker; // lock only those channels being triggered
        p2p::auto_ptr<PVIF> pvif;
        DBEvent evt_VALUE, evt_PROPERTY;
        bool had_initial_VALUE, had_initial_PROPERTY, allowProc;

        Info() :had_initial_VALUE(false), had_initial_PROPERTY(false), allowProc(false) {}
    };
    typedef epics::pvData::shared_vector<Info> members_t;
    members_t members;

    DBManyLock locker; // all member channels

    // monitor only
    epics::pvData::BitSet scratch;

    epics::pvData::PVStructurePtr complete; // complete copy from subscription

    typedef std::set<std::tr1::shared_ptr<PDBGroupMonitor> > interested_t;
    interested_t interested;
    size_t initial_waits;

    static size_t num_instances;

    PDBGroupPV();
    virtual ~PDBGroupPV();

    virtual
    epics::pvAccess::Channel::shared_pointer
        connect(const std::tr1::shared_ptr<PDBProvider>& prov,
                const epics::pvAccess::ChannelRequester::shared_pointer& req);
};

struct epicsShareClass PDBGroupChannel : public BaseChannel,
        public std::tr1::enable_shared_from_this<PDBGroupChannel>
{
    POINTER_DEFINITIONS(PDBGroupChannel);

    PDBGroupPV::shared_pointer pv;

    static size_t num_instances;

    PDBGroupChannel(const PDBGroupPV::shared_pointer& pv,
                const std::tr1::shared_ptr<epics::pvAccess::ChannelProvider>& prov,
                const epics::pvAccess::ChannelRequester::shared_pointer& req);
    virtual ~PDBGroupChannel();

    virtual epics::pvAccess::ChannelPut::shared_pointer createChannelPut(
            epics::pvAccess::ChannelPutRequester::shared_pointer const & requester,
            epics::pvData::PVStructure::shared_pointer const & pvRequest);
    virtual epics::pvData::Monitor::shared_pointer createMonitor(
            epics::pvData::MonitorRequester::shared_pointer const & requester,
            epics::pvData::PVStructure::shared_pointer const & pvRequest);

    virtual void printInfo(std::ostream& out);
};

struct PDBGroupPut : public epics::pvAccess::ChannelPut,
        public std::tr1::enable_shared_from_this<PDBGroupPut>
{
    typedef epics::pvAccess::ChannelPutRequester requester_t;
    PDBGroupChannel::shared_pointer channel;
    requester_type::weak_pointer requester;

    // effectively const after ctor
    bool atomic, doWait;
    PVIF::proc_t doProc;

    epics::pvData::BitSetPtr changed;
    epics::pvData::PVStructurePtr pvf;
    std::vector<std::tr1::shared_ptr<PVIF> > pvif;

    static size_t num_instances;

    PDBGroupPut(const PDBGroupChannel::shared_pointer &channel,
                const epics::pvAccess::ChannelPutRequester::shared_pointer &requester,
                const epics::pvData::PVStructure::shared_pointer& pvReq);
    virtual ~PDBGroupPut();

    virtual void destroy() { pvif.clear(); channel.reset(); requester.reset(); }
    virtual void lock() {}
    virtual void unlock() {}
    virtual std::tr1::shared_ptr<epics::pvAccess::Channel> getChannel() { return channel; }
    virtual void cancel() {}
    virtual void lastRequest() {}
    virtual void put(
            epics::pvData::PVStructure::shared_pointer const & pvPutStructure,
            epics::pvData::BitSet::shared_pointer const & putBitSet);
    virtual void get();
};

struct PDBGroupMonitor : public BaseMonitor
{
    POINTER_DEFINITIONS(PDBGroupMonitor);

    PDBGroupPV::shared_pointer pv;

    bool atomic;

    static size_t num_instances;

    PDBGroupMonitor(const PDBGroupPV::shared_pointer& pv,
                     const requester_type::weak_pointer& requester,
                     const epics::pvData::PVStructure::shared_pointer& pvReq);
    virtual ~PDBGroupMonitor();

    virtual void onStart();
    virtual void onStop();
    virtual void requestUpdate();

    virtual void destroy();

};

#endif // PDBGROUP_H
