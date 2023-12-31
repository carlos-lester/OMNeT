//
// Generated file, do not edit! Created by nedtool 5.6 from inet/routing/rpl/Rpl.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include "Rpl_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace {
template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)(static_cast<const omnetpp::cObject *>(t));
}

template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && !std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)dynamic_cast<const void *>(t);
}

template <class T> inline
typename std::enable_if<!std::is_polymorphic<T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)static_cast<const void *>(t);
}

}

namespace inet {

// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule to generate operator<< for shared_ptr<T>
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const std::shared_ptr<T>& t) { return out << t.get(); }

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');

    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

class cFigure__ColorDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    cFigure__ColorDescriptor();
    virtual ~cFigure__ColorDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(cFigure__ColorDescriptor)

cFigure__ColorDescriptor::cFigure__ColorDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::cFigure::Color)), "")
{
    propertynames = nullptr;
}

cFigure__ColorDescriptor::~cFigure__ColorDescriptor()
{
    delete[] propertynames;
}

bool cFigure__ColorDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<cFigure::Color *>(obj)!=nullptr;
}

const char **cFigure__ColorDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = { "existingClass",  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *cFigure__ColorDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname, "existingClass")) return "";
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int cFigure__ColorDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int cFigure__ColorDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *cFigure__ColorDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int cFigure__ColorDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *cFigure__ColorDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **cFigure__ColorDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *cFigure__ColorDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int cFigure__ColorDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    cFigure::Color *pp = (cFigure::Color *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *cFigure__ColorDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    cFigure::Color *pp = (cFigure::Color *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string cFigure__ColorDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    cFigure::Color *pp = (cFigure::Color *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool cFigure__ColorDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    cFigure::Color *pp = (cFigure::Color *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *cFigure__ColorDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *cFigure__ColorDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    cFigure::Color *pp = (cFigure::Color *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("inet::Ocp");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("inet::Ocp"));
    e->insert(ETX, "ETX");
    e->insert(HOP_COUNT, "HOP_COUNT");
    e->insert(ENERGY, "ENERGY");
    e->insert(HC_MOD, "HC_MOD");
    e->insert(ML, "ML");
    e->insert(RPL_ENH, "RPL_ENH");
    e->insert(RPL_ENH2, "RPL_ENH2");
)

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("inet::RplPacketCode");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("inet::RplPacketCode"));
    e->insert(DIS, "DIS");
    e->insert(DIO, "DIO");
    e->insert(DAO, "DAO");
    e->insert(DAO_ACK, "DAO_ACK");
    e->insert(PING, "PING");
    e->insert(PING_ACK, "PING_ACK");
    e->insert(CROSS_LAYER_CTRL, "CROSS_LAYER_CTRL");
)

Register_Class(RplHeader)

RplHeader::RplHeader() : ::inet::Icmpv6Header()
{
}

RplHeader::RplHeader(const RplHeader& other) : ::inet::Icmpv6Header(other)
{
    copy(other);
}

RplHeader::~RplHeader()
{
}

RplHeader& RplHeader::operator=(const RplHeader& other)
{
    if (this == &other) return *this;
    ::inet::Icmpv6Header::operator=(other);
    copy(other);
    return *this;
}

void RplHeader::copy(const RplHeader& other)
{
    this->icmpv6Code = other.icmpv6Code;
}

void RplHeader::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::Icmpv6Header::parsimPack(b);
    doParsimPacking(b,this->icmpv6Code);
    doParsimPacking(b,this->icmpv6Type);
}

void RplHeader::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::Icmpv6Header::parsimUnpack(b);
    doParsimUnpacking(b,this->icmpv6Code);
    doParsimUnpacking(b,this->icmpv6Type);
}

inet::RplPacketCode RplHeader::getIcmpv6Code() const
{
    return this->icmpv6Code;
}

void RplHeader::setIcmpv6Code(inet::RplPacketCode icmpv6Code)
{
    handleChange();
    this->icmpv6Code = icmpv6Code;
}

uint8_t RplHeader::getIcmpv6Type() const
{
    return this->icmpv6Type;
}

class RplHeaderDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_icmpv6Code,
        FIELD_icmpv6Type,
    };
  public:
    RplHeaderDescriptor();
    virtual ~RplHeaderDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(RplHeaderDescriptor)

RplHeaderDescriptor::RplHeaderDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::RplHeader)), "inet::Icmpv6Header")
{
    propertynames = nullptr;
}

RplHeaderDescriptor::~RplHeaderDescriptor()
{
    delete[] propertynames;
}

bool RplHeaderDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RplHeader *>(obj)!=nullptr;
}

const char **RplHeaderDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *RplHeaderDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int RplHeaderDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount() : 2;
}

unsigned int RplHeaderDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_icmpv6Code
        0,    // FIELD_icmpv6Type
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *RplHeaderDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "icmpv6Code",
        "icmpv6Type",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int RplHeaderDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'i' && strcmp(fieldName, "icmpv6Code") == 0) return base+0;
    if (fieldName[0] == 'i' && strcmp(fieldName, "icmpv6Type") == 0) return base+1;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *RplHeaderDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "inet::RplPacketCode",    // FIELD_icmpv6Code
        "uint8_t",    // FIELD_icmpv6Type
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **RplHeaderDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_icmpv6Code: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *RplHeaderDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_icmpv6Code:
            if (!strcmp(propertyname, "enum")) return "inet::RplPacketCode";
            return nullptr;
        default: return nullptr;
    }
}

int RplHeaderDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    RplHeader *pp = (RplHeader *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *RplHeaderDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RplHeader *pp = (RplHeader *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RplHeaderDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RplHeader *pp = (RplHeader *)object; (void)pp;
    switch (field) {
        case FIELD_icmpv6Code: return enum2string(pp->getIcmpv6Code(), "inet::RplPacketCode");
        case FIELD_icmpv6Type: return ulong2string(pp->getIcmpv6Type());
        default: return "";
    }
}

bool RplHeaderDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    RplHeader *pp = (RplHeader *)object; (void)pp;
    switch (field) {
        case FIELD_icmpv6Code: pp->setIcmpv6Code((inet::RplPacketCode)string2enum(value, "inet::RplPacketCode")); return true;
        default: return false;
    }
}

const char *RplHeaderDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *RplHeaderDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    RplHeader *pp = (RplHeader *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(RplPacket)

RplPacket::RplPacket() : ::inet::FieldsChunk()
{
}

RplPacket::RplPacket(const RplPacket& other) : ::inet::FieldsChunk(other)
{
    copy(other);
}

RplPacket::~RplPacket()
{
}

RplPacket& RplPacket::operator=(const RplPacket& other)
{
    if (this == &other) return *this;
    ::inet::FieldsChunk::operator=(other);
    copy(other);
    return *this;
}

void RplPacket::copy(const RplPacket& other)
{
    this->destAddress = other.destAddress;
    this->srcAddress = other.srcAddress;
    this->instanceId = other.instanceId;
    this->dodagId = other.dodagId;
    this->nodeId = other.nodeId;
}

void RplPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::FieldsChunk::parsimPack(b);
    doParsimPacking(b,this->destAddress);
    doParsimPacking(b,this->srcAddress);
    doParsimPacking(b,this->instanceId);
    doParsimPacking(b,this->dodagId);
    doParsimPacking(b,this->nodeId);
}

void RplPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::FieldsChunk::parsimUnpack(b);
    doParsimUnpacking(b,this->destAddress);
    doParsimUnpacking(b,this->srcAddress);
    doParsimUnpacking(b,this->instanceId);
    doParsimUnpacking(b,this->dodagId);
    doParsimUnpacking(b,this->nodeId);
}

const Ipv6Address& RplPacket::getDestAddress() const
{
    return this->destAddress;
}

void RplPacket::setDestAddress(const Ipv6Address& destAddress)
{
    handleChange();
    this->destAddress = destAddress;
}

const Ipv6Address& RplPacket::getSrcAddress() const
{
    return this->srcAddress;
}

void RplPacket::setSrcAddress(const Ipv6Address& srcAddress)
{
    handleChange();
    this->srcAddress = srcAddress;
}

uint8_t RplPacket::getInstanceId() const
{
    return this->instanceId;
}

void RplPacket::setInstanceId(uint8_t instanceId)
{
    handleChange();
    this->instanceId = instanceId;
}

const Ipv6Address& RplPacket::getDodagId() const
{
    return this->dodagId;
}

void RplPacket::setDodagId(const Ipv6Address& dodagId)
{
    handleChange();
    this->dodagId = dodagId;
}

uint64_t RplPacket::getNodeId() const
{
    return this->nodeId;
}

void RplPacket::setNodeId(uint64_t nodeId)
{
    handleChange();
    this->nodeId = nodeId;
}

class RplPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_destAddress,
        FIELD_srcAddress,
        FIELD_instanceId,
        FIELD_dodagId,
        FIELD_nodeId,
    };
  public:
    RplPacketDescriptor();
    virtual ~RplPacketDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(RplPacketDescriptor)

RplPacketDescriptor::RplPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::RplPacket)), "inet::FieldsChunk")
{
    propertynames = nullptr;
}

RplPacketDescriptor::~RplPacketDescriptor()
{
    delete[] propertynames;
}

bool RplPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RplPacket *>(obj)!=nullptr;
}

const char **RplPacketDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *RplPacketDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int RplPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int RplPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        0,    // FIELD_destAddress
        0,    // FIELD_srcAddress
        FD_ISEDITABLE,    // FIELD_instanceId
        0,    // FIELD_dodagId
        FD_ISEDITABLE,    // FIELD_nodeId
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *RplPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "destAddress",
        "srcAddress",
        "instanceId",
        "dodagId",
        "nodeId",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int RplPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'd' && strcmp(fieldName, "destAddress") == 0) return base+0;
    if (fieldName[0] == 's' && strcmp(fieldName, "srcAddress") == 0) return base+1;
    if (fieldName[0] == 'i' && strcmp(fieldName, "instanceId") == 0) return base+2;
    if (fieldName[0] == 'd' && strcmp(fieldName, "dodagId") == 0) return base+3;
    if (fieldName[0] == 'n' && strcmp(fieldName, "nodeId") == 0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *RplPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "inet::Ipv6Address",    // FIELD_destAddress
        "inet::Ipv6Address",    // FIELD_srcAddress
        "uint8_t",    // FIELD_instanceId
        "inet::Ipv6Address",    // FIELD_dodagId
        "uint64_t",    // FIELD_nodeId
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **RplPacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RplPacketDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RplPacketDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    RplPacket *pp = (RplPacket *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *RplPacketDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RplPacket *pp = (RplPacket *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RplPacketDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RplPacket *pp = (RplPacket *)object; (void)pp;
    switch (field) {
        case FIELD_destAddress: return pp->getDestAddress().str();
        case FIELD_srcAddress: return pp->getSrcAddress().str();
        case FIELD_instanceId: return ulong2string(pp->getInstanceId());
        case FIELD_dodagId: return pp->getDodagId().str();
        case FIELD_nodeId: return uint642string(pp->getNodeId());
        default: return "";
    }
}

bool RplPacketDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    RplPacket *pp = (RplPacket *)object; (void)pp;
    switch (field) {
        case FIELD_instanceId: pp->setInstanceId(string2ulong(value)); return true;
        case FIELD_nodeId: pp->setNodeId(string2uint64(value)); return true;
        default: return false;
    }
}

const char *RplPacketDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *RplPacketDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    RplPacket *pp = (RplPacket *)object; (void)pp;
    switch (field) {
        case FIELD_destAddress: return toVoidPtr(&pp->getDestAddress()); break;
        case FIELD_srcAddress: return toVoidPtr(&pp->getSrcAddress()); break;
        case FIELD_dodagId: return toVoidPtr(&pp->getDodagId()); break;
        default: return nullptr;
    }
}

Register_Class(Dio)

Dio::Dio() : ::inet::RplPacket()
{
}

Dio::Dio(const Dio& other) : ::inet::RplPacket(other)
{
    copy(other);
}

Dio::~Dio()
{
}

Dio& Dio::operator=(const Dio& other)
{
    if (this == &other) return *this;
    ::inet::RplPacket::operator=(other);
    copy(other);
    return *this;
}

void Dio::copy(const Dio& other)
{
    this->dodagVersion = other.dodagVersion;
    this->rank = other.rank;
    this->storing = other.storing;
    this->grounded = other.grounded;
    this->dtsn = other.dtsn;
    this->dioId = other.dioId;
    this->minInterval = other.minInterval;
    this->dioRedundancyConst = other.dioRedundancyConst;
    this->dioNumDoublings = other.dioNumDoublings;
    this->ocp = other.ocp;
    this->position = other.position;
    this->color = other.color;
    this->colorId = other.colorId;
    this->numDIO = other.numDIO;
    this->HC = other.HC;
    this->ETX = other.ETX;
    this->dropB = other.dropB;
    this->dropR = other.dropR;
    this->missACK = other.missACK;
    this->txF = other.txF;
    this->rxF = other.rxF;
    this->bw = other.bw;
    this->den = other.den;
    this->qu = other.qu;
    this->fps = other.fps;
    this->snr = other.snr;
    this->rx_un_suc = other.rx_un_suc;
    this->rx_suc_rate = other.rx_suc_rate;
    this->path_cost = other.path_cost;
    this->last_update = other.last_update;
    this->numDIOrx = other.numDIOrx;
}

void Dio::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::RplPacket::parsimPack(b);
    doParsimPacking(b,this->dodagVersion);
    doParsimPacking(b,this->rank);
    doParsimPacking(b,this->storing);
    doParsimPacking(b,this->grounded);
    doParsimPacking(b,this->dtsn);
    doParsimPacking(b,this->dioId);
    doParsimPacking(b,this->minInterval);
    doParsimPacking(b,this->dioRedundancyConst);
    doParsimPacking(b,this->dioNumDoublings);
    doParsimPacking(b,this->ocp);
    doParsimPacking(b,this->position);
    doParsimPacking(b,this->color);
    doParsimPacking(b,this->colorId);
    doParsimPacking(b,this->numDIO);
    doParsimPacking(b,this->HC);
    doParsimPacking(b,this->ETX);
    doParsimPacking(b,this->dropB);
    doParsimPacking(b,this->dropR);
    doParsimPacking(b,this->missACK);
    doParsimPacking(b,this->txF);
    doParsimPacking(b,this->rxF);
    doParsimPacking(b,this->bw);
    doParsimPacking(b,this->den);
    doParsimPacking(b,this->qu);
    doParsimPacking(b,this->fps);
    doParsimPacking(b,this->snr);
    doParsimPacking(b,this->rx_un_suc);
    doParsimPacking(b,this->rx_suc_rate);
    doParsimPacking(b,this->path_cost);
    doParsimPacking(b,this->last_update);
    doParsimPacking(b,this->numDIOrx);
}

void Dio::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::RplPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->dodagVersion);
    doParsimUnpacking(b,this->rank);
    doParsimUnpacking(b,this->storing);
    doParsimUnpacking(b,this->grounded);
    doParsimUnpacking(b,this->dtsn);
    doParsimUnpacking(b,this->dioId);
    doParsimUnpacking(b,this->minInterval);
    doParsimUnpacking(b,this->dioRedundancyConst);
    doParsimUnpacking(b,this->dioNumDoublings);
    doParsimUnpacking(b,this->ocp);
    doParsimUnpacking(b,this->position);
    doParsimUnpacking(b,this->color);
    doParsimUnpacking(b,this->colorId);
    doParsimUnpacking(b,this->numDIO);
    doParsimUnpacking(b,this->HC);
    doParsimUnpacking(b,this->ETX);
    doParsimUnpacking(b,this->dropB);
    doParsimUnpacking(b,this->dropR);
    doParsimUnpacking(b,this->missACK);
    doParsimUnpacking(b,this->txF);
    doParsimUnpacking(b,this->rxF);
    doParsimUnpacking(b,this->bw);
    doParsimUnpacking(b,this->den);
    doParsimUnpacking(b,this->qu);
    doParsimUnpacking(b,this->fps);
    doParsimUnpacking(b,this->snr);
    doParsimUnpacking(b,this->rx_un_suc);
    doParsimUnpacking(b,this->rx_suc_rate);
    doParsimUnpacking(b,this->path_cost);
    doParsimUnpacking(b,this->last_update);
    doParsimUnpacking(b,this->numDIOrx);
}

uint8_t Dio::getDodagVersion() const
{
    return this->dodagVersion;
}

void Dio::setDodagVersion(uint8_t dodagVersion)
{
    handleChange();
    this->dodagVersion = dodagVersion;
}

double Dio::getRank() const
{
    return this->rank;
}

void Dio::setRank(double rank)
{
    handleChange();
    this->rank = rank;
}

bool Dio::getStoring() const
{
    return this->storing;
}

void Dio::setStoring(bool storing)
{
    handleChange();
    this->storing = storing;
}

bool Dio::getGrounded() const
{
    return this->grounded;
}

void Dio::setGrounded(bool grounded)
{
    handleChange();
    this->grounded = grounded;
}

uint8_t Dio::getDtsn() const
{
    return this->dtsn;
}

void Dio::setDtsn(uint8_t dtsn)
{
    handleChange();
    this->dtsn = dtsn;
}

uint8_t Dio::getDioId() const
{
    return this->dioId;
}

void Dio::setDioId(uint8_t dioId)
{
    handleChange();
    this->dioId = dioId;
}

int Dio::getMinInterval() const
{
    return this->minInterval;
}

void Dio::setMinInterval(int minInterval)
{
    handleChange();
    this->minInterval = minInterval;
}

int Dio::getDioRedundancyConst() const
{
    return this->dioRedundancyConst;
}

void Dio::setDioRedundancyConst(int dioRedundancyConst)
{
    handleChange();
    this->dioRedundancyConst = dioRedundancyConst;
}

int Dio::getDioNumDoublings() const
{
    return this->dioNumDoublings;
}

void Dio::setDioNumDoublings(int dioNumDoublings)
{
    handleChange();
    this->dioNumDoublings = dioNumDoublings;
}

inet::Ocp Dio::getOcp() const
{
    return this->ocp;
}

void Dio::setOcp(inet::Ocp ocp)
{
    handleChange();
    this->ocp = ocp;
}

const Coord& Dio::getPosition() const
{
    return this->position;
}

void Dio::setPosition(const Coord& position)
{
    handleChange();
    this->position = position;
}

const cFigure::Color& Dio::getColor() const
{
    return this->color;
}

void Dio::setColor(const cFigure::Color& color)
{
    handleChange();
    this->color = color;
}

int Dio::getColorId() const
{
    return this->colorId;
}

void Dio::setColorId(int colorId)
{
    handleChange();
    this->colorId = colorId;
}

int Dio::getNumDIO() const
{
    return this->numDIO;
}

void Dio::setNumDIO(int numDIO)
{
    handleChange();
    this->numDIO = numDIO;
}

int Dio::getHC() const
{
    return this->HC;
}

void Dio::setHC(int HC)
{
    handleChange();
    this->HC = HC;
}

double Dio::getETX() const
{
    return this->ETX;
}

void Dio::setETX(double ETX)
{
    handleChange();
    this->ETX = ETX;
}

int Dio::getDropB() const
{
    return this->dropB;
}

void Dio::setDropB(int dropB)
{
    handleChange();
    this->dropB = dropB;
}

int Dio::getDropR() const
{
    return this->dropR;
}

void Dio::setDropR(int dropR)
{
    handleChange();
    this->dropR = dropR;
}

int Dio::getMissACK() const
{
    return this->missACK;
}

void Dio::setMissACK(int missACK)
{
    handleChange();
    this->missACK = missACK;
}

double Dio::getTxF() const
{
    return this->txF;
}

void Dio::setTxF(double txF)
{
    handleChange();
    this->txF = txF;
}

double Dio::getRxF() const
{
    return this->rxF;
}

void Dio::setRxF(double rxF)
{
    handleChange();
    this->rxF = rxF;
}

double Dio::getBw() const
{
    return this->bw;
}

void Dio::setBw(double bw)
{
    handleChange();
    this->bw = bw;
}

int Dio::getDen() const
{
    return this->den;
}

void Dio::setDen(int den)
{
    handleChange();
    this->den = den;
}

double Dio::getQu() const
{
    return this->qu;
}

void Dio::setQu(double qu)
{
    handleChange();
    this->qu = qu;
}

double Dio::getFps() const
{
    return this->fps;
}

void Dio::setFps(double fps)
{
    handleChange();
    this->fps = fps;
}

double Dio::getSnr() const
{
    return this->snr;
}

void Dio::setSnr(double snr)
{
    handleChange();
    this->snr = snr;
}

int Dio::getRx_un_suc() const
{
    return this->rx_un_suc;
}

void Dio::setRx_un_suc(int rx_un_suc)
{
    handleChange();
    this->rx_un_suc = rx_un_suc;
}

double Dio::getRx_suc_rate() const
{
    return this->rx_suc_rate;
}

void Dio::setRx_suc_rate(double rx_suc_rate)
{
    handleChange();
    this->rx_suc_rate = rx_suc_rate;
}

double Dio::getPath_cost() const
{
    return this->path_cost;
}

void Dio::setPath_cost(double path_cost)
{
    handleChange();
    this->path_cost = path_cost;
}

omnetpp::simtime_t Dio::getLast_update() const
{
    return this->last_update;
}

void Dio::setLast_update(omnetpp::simtime_t last_update)
{
    handleChange();
    this->last_update = last_update;
}

int Dio::getNumDIOrx() const
{
    return this->numDIOrx;
}

void Dio::setNumDIOrx(int numDIOrx)
{
    handleChange();
    this->numDIOrx = numDIOrx;
}

class DioDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_dodagVersion,
        FIELD_rank,
        FIELD_storing,
        FIELD_grounded,
        FIELD_dtsn,
        FIELD_dioId,
        FIELD_minInterval,
        FIELD_dioRedundancyConst,
        FIELD_dioNumDoublings,
        FIELD_ocp,
        FIELD_position,
        FIELD_color,
        FIELD_colorId,
        FIELD_numDIO,
        FIELD_HC,
        FIELD_ETX,
        FIELD_dropB,
        FIELD_dropR,
        FIELD_missACK,
        FIELD_txF,
        FIELD_rxF,
        FIELD_bw,
        FIELD_den,
        FIELD_qu,
        FIELD_fps,
        FIELD_snr,
        FIELD_rx_un_suc,
        FIELD_rx_suc_rate,
        FIELD_path_cost,
        FIELD_last_update,
        FIELD_numDIOrx,
    };
  public:
    DioDescriptor();
    virtual ~DioDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(DioDescriptor)

DioDescriptor::DioDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::Dio)), "inet::RplPacket")
{
    propertynames = nullptr;
}

DioDescriptor::~DioDescriptor()
{
    delete[] propertynames;
}

bool DioDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<Dio *>(obj)!=nullptr;
}

const char **DioDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *DioDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int DioDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 31+basedesc->getFieldCount() : 31;
}

unsigned int DioDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_dodagVersion
        FD_ISEDITABLE,    // FIELD_rank
        FD_ISEDITABLE,    // FIELD_storing
        FD_ISEDITABLE,    // FIELD_grounded
        FD_ISEDITABLE,    // FIELD_dtsn
        FD_ISEDITABLE,    // FIELD_dioId
        FD_ISEDITABLE,    // FIELD_minInterval
        FD_ISEDITABLE,    // FIELD_dioRedundancyConst
        FD_ISEDITABLE,    // FIELD_dioNumDoublings
        FD_ISEDITABLE,    // FIELD_ocp
        FD_ISCOMPOUND,    // FIELD_position
        FD_ISCOMPOUND,    // FIELD_color
        FD_ISEDITABLE,    // FIELD_colorId
        FD_ISEDITABLE,    // FIELD_numDIO
        FD_ISEDITABLE,    // FIELD_HC
        FD_ISEDITABLE,    // FIELD_ETX
        FD_ISEDITABLE,    // FIELD_dropB
        FD_ISEDITABLE,    // FIELD_dropR
        FD_ISEDITABLE,    // FIELD_missACK
        FD_ISEDITABLE,    // FIELD_txF
        FD_ISEDITABLE,    // FIELD_rxF
        FD_ISEDITABLE,    // FIELD_bw
        FD_ISEDITABLE,    // FIELD_den
        FD_ISEDITABLE,    // FIELD_qu
        FD_ISEDITABLE,    // FIELD_fps
        FD_ISEDITABLE,    // FIELD_snr
        FD_ISEDITABLE,    // FIELD_rx_un_suc
        FD_ISEDITABLE,    // FIELD_rx_suc_rate
        FD_ISEDITABLE,    // FIELD_path_cost
        0,    // FIELD_last_update
        FD_ISEDITABLE,    // FIELD_numDIOrx
    };
    return (field >= 0 && field < 31) ? fieldTypeFlags[field] : 0;
}

const char *DioDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dodagVersion",
        "rank",
        "storing",
        "grounded",
        "dtsn",
        "dioId",
        "minInterval",
        "dioRedundancyConst",
        "dioNumDoublings",
        "ocp",
        "position",
        "color",
        "colorId",
        "numDIO",
        "HC",
        "ETX",
        "dropB",
        "dropR",
        "missACK",
        "txF",
        "rxF",
        "bw",
        "den",
        "qu",
        "fps",
        "snr",
        "rx_un_suc",
        "rx_suc_rate",
        "path_cost",
        "last_update",
        "numDIOrx",
    };
    return (field >= 0 && field < 31) ? fieldNames[field] : nullptr;
}

int DioDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'd' && strcmp(fieldName, "dodagVersion") == 0) return base+0;
    if (fieldName[0] == 'r' && strcmp(fieldName, "rank") == 0) return base+1;
    if (fieldName[0] == 's' && strcmp(fieldName, "storing") == 0) return base+2;
    if (fieldName[0] == 'g' && strcmp(fieldName, "grounded") == 0) return base+3;
    if (fieldName[0] == 'd' && strcmp(fieldName, "dtsn") == 0) return base+4;
    if (fieldName[0] == 'd' && strcmp(fieldName, "dioId") == 0) return base+5;
    if (fieldName[0] == 'm' && strcmp(fieldName, "minInterval") == 0) return base+6;
    if (fieldName[0] == 'd' && strcmp(fieldName, "dioRedundancyConst") == 0) return base+7;
    if (fieldName[0] == 'd' && strcmp(fieldName, "dioNumDoublings") == 0) return base+8;
    if (fieldName[0] == 'o' && strcmp(fieldName, "ocp") == 0) return base+9;
    if (fieldName[0] == 'p' && strcmp(fieldName, "position") == 0) return base+10;
    if (fieldName[0] == 'c' && strcmp(fieldName, "color") == 0) return base+11;
    if (fieldName[0] == 'c' && strcmp(fieldName, "colorId") == 0) return base+12;
    if (fieldName[0] == 'n' && strcmp(fieldName, "numDIO") == 0) return base+13;
    if (fieldName[0] == 'H' && strcmp(fieldName, "HC") == 0) return base+14;
    if (fieldName[0] == 'E' && strcmp(fieldName, "ETX") == 0) return base+15;
    if (fieldName[0] == 'd' && strcmp(fieldName, "dropB") == 0) return base+16;
    if (fieldName[0] == 'd' && strcmp(fieldName, "dropR") == 0) return base+17;
    if (fieldName[0] == 'm' && strcmp(fieldName, "missACK") == 0) return base+18;
    if (fieldName[0] == 't' && strcmp(fieldName, "txF") == 0) return base+19;
    if (fieldName[0] == 'r' && strcmp(fieldName, "rxF") == 0) return base+20;
    if (fieldName[0] == 'b' && strcmp(fieldName, "bw") == 0) return base+21;
    if (fieldName[0] == 'd' && strcmp(fieldName, "den") == 0) return base+22;
    if (fieldName[0] == 'q' && strcmp(fieldName, "qu") == 0) return base+23;
    if (fieldName[0] == 'f' && strcmp(fieldName, "fps") == 0) return base+24;
    if (fieldName[0] == 's' && strcmp(fieldName, "snr") == 0) return base+25;
    if (fieldName[0] == 'r' && strcmp(fieldName, "rx_un_suc") == 0) return base+26;
    if (fieldName[0] == 'r' && strcmp(fieldName, "rx_suc_rate") == 0) return base+27;
    if (fieldName[0] == 'p' && strcmp(fieldName, "path_cost") == 0) return base+28;
    if (fieldName[0] == 'l' && strcmp(fieldName, "last_update") == 0) return base+29;
    if (fieldName[0] == 'n' && strcmp(fieldName, "numDIOrx") == 0) return base+30;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *DioDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "uint8_t",    // FIELD_dodagVersion
        "double",    // FIELD_rank
        "bool",    // FIELD_storing
        "bool",    // FIELD_grounded
        "uint8_t",    // FIELD_dtsn
        "uint8_t",    // FIELD_dioId
        "int",    // FIELD_minInterval
        "int",    // FIELD_dioRedundancyConst
        "int",    // FIELD_dioNumDoublings
        "inet::Ocp",    // FIELD_ocp
        "inet::Coord",    // FIELD_position
        "inet::cFigure::Color",    // FIELD_color
        "int",    // FIELD_colorId
        "int",    // FIELD_numDIO
        "int",    // FIELD_HC
        "double",    // FIELD_ETX
        "int",    // FIELD_dropB
        "int",    // FIELD_dropR
        "int",    // FIELD_missACK
        "double",    // FIELD_txF
        "double",    // FIELD_rxF
        "double",    // FIELD_bw
        "int",    // FIELD_den
        "double",    // FIELD_qu
        "double",    // FIELD_fps
        "double",    // FIELD_snr
        "int",    // FIELD_rx_un_suc
        "double",    // FIELD_rx_suc_rate
        "double",    // FIELD_path_cost
        "omnetpp::simtime_t",    // FIELD_last_update
        "int",    // FIELD_numDIOrx
    };
    return (field >= 0 && field < 31) ? fieldTypeStrings[field] : nullptr;
}

const char **DioDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_ocp: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *DioDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_ocp:
            if (!strcmp(propertyname, "enum")) return "inet::Ocp";
            return nullptr;
        default: return nullptr;
    }
}

int DioDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    Dio *pp = (Dio *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *DioDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Dio *pp = (Dio *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string DioDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Dio *pp = (Dio *)object; (void)pp;
    switch (field) {
        case FIELD_dodagVersion: return ulong2string(pp->getDodagVersion());
        case FIELD_rank: return double2string(pp->getRank());
        case FIELD_storing: return bool2string(pp->getStoring());
        case FIELD_grounded: return bool2string(pp->getGrounded());
        case FIELD_dtsn: return ulong2string(pp->getDtsn());
        case FIELD_dioId: return ulong2string(pp->getDioId());
        case FIELD_minInterval: return long2string(pp->getMinInterval());
        case FIELD_dioRedundancyConst: return long2string(pp->getDioRedundancyConst());
        case FIELD_dioNumDoublings: return long2string(pp->getDioNumDoublings());
        case FIELD_ocp: return enum2string(pp->getOcp(), "inet::Ocp");
        case FIELD_position: {std::stringstream out; out << pp->getPosition(); return out.str();}
        case FIELD_color: {std::stringstream out; out << pp->getColor(); return out.str();}
        case FIELD_colorId: return long2string(pp->getColorId());
        case FIELD_numDIO: return long2string(pp->getNumDIO());
        case FIELD_HC: return long2string(pp->getHC());
        case FIELD_ETX: return double2string(pp->getETX());
        case FIELD_dropB: return long2string(pp->getDropB());
        case FIELD_dropR: return long2string(pp->getDropR());
        case FIELD_missACK: return long2string(pp->getMissACK());
        case FIELD_txF: return double2string(pp->getTxF());
        case FIELD_rxF: return double2string(pp->getRxF());
        case FIELD_bw: return double2string(pp->getBw());
        case FIELD_den: return long2string(pp->getDen());
        case FIELD_qu: return double2string(pp->getQu());
        case FIELD_fps: return double2string(pp->getFps());
        case FIELD_snr: return double2string(pp->getSnr());
        case FIELD_rx_un_suc: return long2string(pp->getRx_un_suc());
        case FIELD_rx_suc_rate: return double2string(pp->getRx_suc_rate());
        case FIELD_path_cost: return double2string(pp->getPath_cost());
        case FIELD_last_update: return simtime2string(pp->getLast_update());
        case FIELD_numDIOrx: return long2string(pp->getNumDIOrx());
        default: return "";
    }
}

bool DioDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    Dio *pp = (Dio *)object; (void)pp;
    switch (field) {
        case FIELD_dodagVersion: pp->setDodagVersion(string2ulong(value)); return true;
        case FIELD_rank: pp->setRank(string2double(value)); return true;
        case FIELD_storing: pp->setStoring(string2bool(value)); return true;
        case FIELD_grounded: pp->setGrounded(string2bool(value)); return true;
        case FIELD_dtsn: pp->setDtsn(string2ulong(value)); return true;
        case FIELD_dioId: pp->setDioId(string2ulong(value)); return true;
        case FIELD_minInterval: pp->setMinInterval(string2long(value)); return true;
        case FIELD_dioRedundancyConst: pp->setDioRedundancyConst(string2long(value)); return true;
        case FIELD_dioNumDoublings: pp->setDioNumDoublings(string2long(value)); return true;
        case FIELD_ocp: pp->setOcp((inet::Ocp)string2enum(value, "inet::Ocp")); return true;
        case FIELD_colorId: pp->setColorId(string2long(value)); return true;
        case FIELD_numDIO: pp->setNumDIO(string2long(value)); return true;
        case FIELD_HC: pp->setHC(string2long(value)); return true;
        case FIELD_ETX: pp->setETX(string2double(value)); return true;
        case FIELD_dropB: pp->setDropB(string2long(value)); return true;
        case FIELD_dropR: pp->setDropR(string2long(value)); return true;
        case FIELD_missACK: pp->setMissACK(string2long(value)); return true;
        case FIELD_txF: pp->setTxF(string2double(value)); return true;
        case FIELD_rxF: pp->setRxF(string2double(value)); return true;
        case FIELD_bw: pp->setBw(string2double(value)); return true;
        case FIELD_den: pp->setDen(string2long(value)); return true;
        case FIELD_qu: pp->setQu(string2double(value)); return true;
        case FIELD_fps: pp->setFps(string2double(value)); return true;
        case FIELD_snr: pp->setSnr(string2double(value)); return true;
        case FIELD_rx_un_suc: pp->setRx_un_suc(string2long(value)); return true;
        case FIELD_rx_suc_rate: pp->setRx_suc_rate(string2double(value)); return true;
        case FIELD_path_cost: pp->setPath_cost(string2double(value)); return true;
        case FIELD_numDIOrx: pp->setNumDIOrx(string2long(value)); return true;
        default: return false;
    }
}

const char *DioDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_position: return omnetpp::opp_typename(typeid(Coord));
        case FIELD_color: return omnetpp::opp_typename(typeid(cFigure::Color));
        default: return nullptr;
    };
}

void *DioDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    Dio *pp = (Dio *)object; (void)pp;
    switch (field) {
        case FIELD_position: return toVoidPtr(&pp->getPosition()); break;
        case FIELD_color: return toVoidPtr(&pp->getColor()); break;
        default: return nullptr;
    }
}

Register_Class(Dao)

Dao::Dao() : ::inet::RplPacket()
{
}

Dao::Dao(const Dao& other) : ::inet::RplPacket(other)
{
    copy(other);
}

Dao::~Dao()
{
}

Dao& Dao::operator=(const Dao& other)
{
    if (this == &other) return *this;
    ::inet::RplPacket::operator=(other);
    copy(other);
    return *this;
}

void Dao::copy(const Dao& other)
{
    this->seqNum = other.seqNum;
    this->daoAckRequired = other.daoAckRequired;
    this->reachableDest = other.reachableDest;
    this->chOffset = other.chOffset;
}

void Dao::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::RplPacket::parsimPack(b);
    doParsimPacking(b,this->seqNum);
    doParsimPacking(b,this->daoAckRequired);
    doParsimPacking(b,this->reachableDest);
    doParsimPacking(b,this->chOffset);
}

void Dao::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::RplPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->seqNum);
    doParsimUnpacking(b,this->daoAckRequired);
    doParsimUnpacking(b,this->reachableDest);
    doParsimUnpacking(b,this->chOffset);
}

uint8_t Dao::getSeqNum() const
{
    return this->seqNum;
}

void Dao::setSeqNum(uint8_t seqNum)
{
    handleChange();
    this->seqNum = seqNum;
}

bool Dao::getDaoAckRequired() const
{
    return this->daoAckRequired;
}

void Dao::setDaoAckRequired(bool daoAckRequired)
{
    handleChange();
    this->daoAckRequired = daoAckRequired;
}

const Ipv6Address& Dao::getReachableDest() const
{
    return this->reachableDest;
}

void Dao::setReachableDest(const Ipv6Address& reachableDest)
{
    handleChange();
    this->reachableDest = reachableDest;
}

uint8_t Dao::getChOffset() const
{
    return this->chOffset;
}

void Dao::setChOffset(uint8_t chOffset)
{
    handleChange();
    this->chOffset = chOffset;
}

class DaoDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_seqNum,
        FIELD_daoAckRequired,
        FIELD_reachableDest,
        FIELD_chOffset,
    };
  public:
    DaoDescriptor();
    virtual ~DaoDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(DaoDescriptor)

DaoDescriptor::DaoDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::Dao)), "inet::RplPacket")
{
    propertynames = nullptr;
}

DaoDescriptor::~DaoDescriptor()
{
    delete[] propertynames;
}

bool DaoDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<Dao *>(obj)!=nullptr;
}

const char **DaoDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *DaoDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int DaoDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount() : 4;
}

unsigned int DaoDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_seqNum
        FD_ISEDITABLE,    // FIELD_daoAckRequired
        0,    // FIELD_reachableDest
        FD_ISEDITABLE,    // FIELD_chOffset
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *DaoDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "seqNum",
        "daoAckRequired",
        "reachableDest",
        "chOffset",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int DaoDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 's' && strcmp(fieldName, "seqNum") == 0) return base+0;
    if (fieldName[0] == 'd' && strcmp(fieldName, "daoAckRequired") == 0) return base+1;
    if (fieldName[0] == 'r' && strcmp(fieldName, "reachableDest") == 0) return base+2;
    if (fieldName[0] == 'c' && strcmp(fieldName, "chOffset") == 0) return base+3;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *DaoDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "uint8_t",    // FIELD_seqNum
        "bool",    // FIELD_daoAckRequired
        "inet::Ipv6Address",    // FIELD_reachableDest
        "uint8_t",    // FIELD_chOffset
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **DaoDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *DaoDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int DaoDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    Dao *pp = (Dao *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *DaoDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Dao *pp = (Dao *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string DaoDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Dao *pp = (Dao *)object; (void)pp;
    switch (field) {
        case FIELD_seqNum: return ulong2string(pp->getSeqNum());
        case FIELD_daoAckRequired: return bool2string(pp->getDaoAckRequired());
        case FIELD_reachableDest: return pp->getReachableDest().str();
        case FIELD_chOffset: return ulong2string(pp->getChOffset());
        default: return "";
    }
}

bool DaoDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    Dao *pp = (Dao *)object; (void)pp;
    switch (field) {
        case FIELD_seqNum: pp->setSeqNum(string2ulong(value)); return true;
        case FIELD_daoAckRequired: pp->setDaoAckRequired(string2bool(value)); return true;
        case FIELD_chOffset: pp->setChOffset(string2ulong(value)); return true;
        default: return false;
    }
}

const char *DaoDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *DaoDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    Dao *pp = (Dao *)object; (void)pp;
    switch (field) {
        case FIELD_reachableDest: return toVoidPtr(&pp->getReachableDest()); break;
        default: return nullptr;
    }
}

Register_Class(Dis)

Dis::Dis() : ::inet::RplPacket()
{
}

Dis::Dis(const Dis& other) : ::inet::RplPacket(other)
{
    copy(other);
}

Dis::~Dis()
{
}

Dis& Dis::operator=(const Dis& other)
{
    if (this == &other) return *this;
    ::inet::RplPacket::operator=(other);
    copy(other);
    return *this;
}

void Dis::copy(const Dis& other)
{
}

void Dis::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::RplPacket::parsimPack(b);
}

void Dis::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::RplPacket::parsimUnpack(b);
}

class DisDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    DisDescriptor();
    virtual ~DisDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(DisDescriptor)

DisDescriptor::DisDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::Dis)), "inet::RplPacket")
{
    propertynames = nullptr;
}

DisDescriptor::~DisDescriptor()
{
    delete[] propertynames;
}

bool DisDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<Dis *>(obj)!=nullptr;
}

const char **DisDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *DisDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int DisDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int DisDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *DisDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int DisDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *DisDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **DisDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *DisDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int DisDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    Dis *pp = (Dis *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *DisDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Dis *pp = (Dis *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string DisDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Dis *pp = (Dis *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool DisDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    Dis *pp = (Dis *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *DisDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *DisDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    Dis *pp = (Dis *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(RplPacketInfo)

RplPacketInfo::RplPacketInfo() : ::inet::RplPacket()
{
}

RplPacketInfo::RplPacketInfo(const RplPacketInfo& other) : ::inet::RplPacket(other)
{
    copy(other);
}

RplPacketInfo::~RplPacketInfo()
{
}

RplPacketInfo& RplPacketInfo::operator=(const RplPacketInfo& other)
{
    if (this == &other) return *this;
    ::inet::RplPacket::operator=(other);
    copy(other);
    return *this;
}

void RplPacketInfo::copy(const RplPacketInfo& other)
{
    this->down = other.down;
    this->rankError = other.rankError;
    this->fwdError = other.fwdError;
    this->senderRank = other.senderRank;
}

void RplPacketInfo::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::RplPacket::parsimPack(b);
    doParsimPacking(b,this->down);
    doParsimPacking(b,this->rankError);
    doParsimPacking(b,this->fwdError);
    doParsimPacking(b,this->senderRank);
}

void RplPacketInfo::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::RplPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->down);
    doParsimUnpacking(b,this->rankError);
    doParsimUnpacking(b,this->fwdError);
    doParsimUnpacking(b,this->senderRank);
}

bool RplPacketInfo::getDown() const
{
    return this->down;
}

void RplPacketInfo::setDown(bool down)
{
    handleChange();
    this->down = down;
}

bool RplPacketInfo::getRankError() const
{
    return this->rankError;
}

void RplPacketInfo::setRankError(bool rankError)
{
    handleChange();
    this->rankError = rankError;
}

bool RplPacketInfo::getFwdError() const
{
    return this->fwdError;
}

void RplPacketInfo::setFwdError(bool fwdError)
{
    handleChange();
    this->fwdError = fwdError;
}

uint16_t RplPacketInfo::getSenderRank() const
{
    return this->senderRank;
}

void RplPacketInfo::setSenderRank(uint16_t senderRank)
{
    handleChange();
    this->senderRank = senderRank;
}

class RplPacketInfoDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_down,
        FIELD_rankError,
        FIELD_fwdError,
        FIELD_senderRank,
    };
  public:
    RplPacketInfoDescriptor();
    virtual ~RplPacketInfoDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(RplPacketInfoDescriptor)

RplPacketInfoDescriptor::RplPacketInfoDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::RplPacketInfo)), "inet::RplPacket")
{
    propertynames = nullptr;
}

RplPacketInfoDescriptor::~RplPacketInfoDescriptor()
{
    delete[] propertynames;
}

bool RplPacketInfoDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RplPacketInfo *>(obj)!=nullptr;
}

const char **RplPacketInfoDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *RplPacketInfoDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int RplPacketInfoDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount() : 4;
}

unsigned int RplPacketInfoDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_down
        FD_ISEDITABLE,    // FIELD_rankError
        FD_ISEDITABLE,    // FIELD_fwdError
        FD_ISEDITABLE,    // FIELD_senderRank
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *RplPacketInfoDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "down",
        "rankError",
        "fwdError",
        "senderRank",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int RplPacketInfoDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'd' && strcmp(fieldName, "down") == 0) return base+0;
    if (fieldName[0] == 'r' && strcmp(fieldName, "rankError") == 0) return base+1;
    if (fieldName[0] == 'f' && strcmp(fieldName, "fwdError") == 0) return base+2;
    if (fieldName[0] == 's' && strcmp(fieldName, "senderRank") == 0) return base+3;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *RplPacketInfoDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_down
        "bool",    // FIELD_rankError
        "bool",    // FIELD_fwdError
        "uint16_t",    // FIELD_senderRank
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **RplPacketInfoDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RplPacketInfoDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RplPacketInfoDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    RplPacketInfo *pp = (RplPacketInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *RplPacketInfoDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RplPacketInfo *pp = (RplPacketInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RplPacketInfoDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RplPacketInfo *pp = (RplPacketInfo *)object; (void)pp;
    switch (field) {
        case FIELD_down: return bool2string(pp->getDown());
        case FIELD_rankError: return bool2string(pp->getRankError());
        case FIELD_fwdError: return bool2string(pp->getFwdError());
        case FIELD_senderRank: return ulong2string(pp->getSenderRank());
        default: return "";
    }
}

bool RplPacketInfoDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    RplPacketInfo *pp = (RplPacketInfo *)object; (void)pp;
    switch (field) {
        case FIELD_down: pp->setDown(string2bool(value)); return true;
        case FIELD_rankError: pp->setRankError(string2bool(value)); return true;
        case FIELD_fwdError: pp->setFwdError(string2bool(value)); return true;
        case FIELD_senderRank: pp->setSenderRank(string2ulong(value)); return true;
        default: return false;
    }
}

const char *RplPacketInfoDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *RplPacketInfoDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    RplPacketInfo *pp = (RplPacketInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(RplTargetInfo)

RplTargetInfo::RplTargetInfo() : ::inet::RplPacket()
{
}

RplTargetInfo::RplTargetInfo(const RplTargetInfo& other) : ::inet::RplPacket(other)
{
    copy(other);
}

RplTargetInfo::~RplTargetInfo()
{
}

RplTargetInfo& RplTargetInfo::operator=(const RplTargetInfo& other)
{
    if (this == &other) return *this;
    ::inet::RplPacket::operator=(other);
    copy(other);
    return *this;
}

void RplTargetInfo::copy(const RplTargetInfo& other)
{
    this->target = other.target;
}

void RplTargetInfo::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::RplPacket::parsimPack(b);
    doParsimPacking(b,this->target);
}

void RplTargetInfo::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::RplPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->target);
}

const Ipv6Address& RplTargetInfo::getTarget() const
{
    return this->target;
}

void RplTargetInfo::setTarget(const Ipv6Address& target)
{
    handleChange();
    this->target = target;
}

class RplTargetInfoDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_target,
    };
  public:
    RplTargetInfoDescriptor();
    virtual ~RplTargetInfoDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(RplTargetInfoDescriptor)

RplTargetInfoDescriptor::RplTargetInfoDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::RplTargetInfo)), "inet::RplPacket")
{
    propertynames = nullptr;
}

RplTargetInfoDescriptor::~RplTargetInfoDescriptor()
{
    delete[] propertynames;
}

bool RplTargetInfoDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RplTargetInfo *>(obj)!=nullptr;
}

const char **RplTargetInfoDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *RplTargetInfoDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int RplTargetInfoDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount() : 1;
}

unsigned int RplTargetInfoDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        0,    // FIELD_target
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *RplTargetInfoDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "target",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int RplTargetInfoDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 't' && strcmp(fieldName, "target") == 0) return base+0;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *RplTargetInfoDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "inet::Ipv6Address",    // FIELD_target
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **RplTargetInfoDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RplTargetInfoDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RplTargetInfoDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    RplTargetInfo *pp = (RplTargetInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *RplTargetInfoDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RplTargetInfo *pp = (RplTargetInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RplTargetInfoDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RplTargetInfo *pp = (RplTargetInfo *)object; (void)pp;
    switch (field) {
        case FIELD_target: return pp->getTarget().str();
        default: return "";
    }
}

bool RplTargetInfoDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    RplTargetInfo *pp = (RplTargetInfo *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *RplTargetInfoDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *RplTargetInfoDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    RplTargetInfo *pp = (RplTargetInfo *)object; (void)pp;
    switch (field) {
        case FIELD_target: return toVoidPtr(&pp->getTarget()); break;
        default: return nullptr;
    }
}

Register_Class(RplTransitInfo)

RplTransitInfo::RplTransitInfo() : ::inet::RplPacket()
{
}

RplTransitInfo::RplTransitInfo(const RplTransitInfo& other) : ::inet::RplPacket(other)
{
    copy(other);
}

RplTransitInfo::~RplTransitInfo()
{
}

RplTransitInfo& RplTransitInfo::operator=(const RplTransitInfo& other)
{
    if (this == &other) return *this;
    ::inet::RplPacket::operator=(other);
    copy(other);
    return *this;
}

void RplTransitInfo::copy(const RplTransitInfo& other)
{
    this->transit = other.transit;
}

void RplTransitInfo::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::RplPacket::parsimPack(b);
    doParsimPacking(b,this->transit);
}

void RplTransitInfo::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::RplPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->transit);
}

const Ipv6Address& RplTransitInfo::getTransit() const
{
    return this->transit;
}

void RplTransitInfo::setTransit(const Ipv6Address& transit)
{
    handleChange();
    this->transit = transit;
}

class RplTransitInfoDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_transit,
    };
  public:
    RplTransitInfoDescriptor();
    virtual ~RplTransitInfoDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(RplTransitInfoDescriptor)

RplTransitInfoDescriptor::RplTransitInfoDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::RplTransitInfo)), "inet::RplPacket")
{
    propertynames = nullptr;
}

RplTransitInfoDescriptor::~RplTransitInfoDescriptor()
{
    delete[] propertynames;
}

bool RplTransitInfoDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RplTransitInfo *>(obj)!=nullptr;
}

const char **RplTransitInfoDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *RplTransitInfoDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int RplTransitInfoDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount() : 1;
}

unsigned int RplTransitInfoDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        0,    // FIELD_transit
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *RplTransitInfoDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "transit",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int RplTransitInfoDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 't' && strcmp(fieldName, "transit") == 0) return base+0;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *RplTransitInfoDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "inet::Ipv6Address",    // FIELD_transit
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **RplTransitInfoDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RplTransitInfoDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RplTransitInfoDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    RplTransitInfo *pp = (RplTransitInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *RplTransitInfoDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RplTransitInfo *pp = (RplTransitInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RplTransitInfoDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RplTransitInfo *pp = (RplTransitInfo *)object; (void)pp;
    switch (field) {
        case FIELD_transit: return pp->getTransit().str();
        default: return "";
    }
}

bool RplTransitInfoDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    RplTransitInfo *pp = (RplTransitInfo *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *RplTransitInfoDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *RplTransitInfoDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    RplTransitInfo *pp = (RplTransitInfo *)object; (void)pp;
    switch (field) {
        case FIELD_transit: return toVoidPtr(&pp->getTransit()); break;
        default: return nullptr;
    }
}

Register_Class(SourceRoutingHeader)

SourceRoutingHeader::SourceRoutingHeader() : ::inet::FieldsChunk()
{
}

SourceRoutingHeader::SourceRoutingHeader(const SourceRoutingHeader& other) : ::inet::FieldsChunk(other)
{
    copy(other);
}

SourceRoutingHeader::~SourceRoutingHeader()
{
}

SourceRoutingHeader& SourceRoutingHeader::operator=(const SourceRoutingHeader& other)
{
    if (this == &other) return *this;
    ::inet::FieldsChunk::operator=(other);
    copy(other);
    return *this;
}

void SourceRoutingHeader::copy(const SourceRoutingHeader& other)
{
}

void SourceRoutingHeader::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::FieldsChunk::parsimPack(b);
}

void SourceRoutingHeader::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::FieldsChunk::parsimUnpack(b);
}

class SourceRoutingHeaderDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    SourceRoutingHeaderDescriptor();
    virtual ~SourceRoutingHeaderDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(SourceRoutingHeaderDescriptor)

SourceRoutingHeaderDescriptor::SourceRoutingHeaderDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::SourceRoutingHeader)), "inet::FieldsChunk")
{
    propertynames = nullptr;
}

SourceRoutingHeaderDescriptor::~SourceRoutingHeaderDescriptor()
{
    delete[] propertynames;
}

bool SourceRoutingHeaderDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<SourceRoutingHeader *>(obj)!=nullptr;
}

const char **SourceRoutingHeaderDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *SourceRoutingHeaderDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int SourceRoutingHeaderDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int SourceRoutingHeaderDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *SourceRoutingHeaderDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int SourceRoutingHeaderDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *SourceRoutingHeaderDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **SourceRoutingHeaderDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *SourceRoutingHeaderDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int SourceRoutingHeaderDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    SourceRoutingHeader *pp = (SourceRoutingHeader *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *SourceRoutingHeaderDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    SourceRoutingHeader *pp = (SourceRoutingHeader *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string SourceRoutingHeaderDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    SourceRoutingHeader *pp = (SourceRoutingHeader *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool SourceRoutingHeaderDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    SourceRoutingHeader *pp = (SourceRoutingHeader *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *SourceRoutingHeaderDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *SourceRoutingHeaderDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    SourceRoutingHeader *pp = (SourceRoutingHeader *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

} // namespace inet

