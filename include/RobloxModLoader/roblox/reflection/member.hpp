#pragma once

#include "descriptor.hpp"
#include "dense_hash.hpp"

class ClassDescriptor;

struct StringHashPredicate {
    size_t operator()(const char *s) const;
};

struct StringEqualPredicate {
    bool operator()(const char *lhs, const char *rhs) const {
        return strcmp(lhs, rhs) == 0;
    }
};

template<typename MemberDescriptorType>
class MemberDescriptorContainer {
    static bool compare(const MemberDescriptorType *a, const MemberDescriptorType *b) {
        return a->Name < b->Name;
    }

public:
    class Collection : public std::vector<MemberDescriptorType *> {
    };

    typedef DenseHashMap<const char *, MemberDescriptorType *, StringHashPredicate, StringEqualPredicate>
    DescriptorLookup;

protected:
    Collection descriptors;
    DescriptorLookup descriptorLookup;

    std::vector<MemberDescriptorContainer *> derivedContainers;
    MemberDescriptorContainer *const base;

private:
    std::byte pad_0000[0x40];

public:
    MemberDescriptorType *find_descriptor(const char *name) const {
        for (auto descriptor: descriptors) {
            if (strcmp(descriptor->name.data(), name) == 0) {
                return descriptor;
            }
        }
        return nullptr;
    }
};

class MemberDescriptor : public Descriptor {
public:
    static void (*member_hiding_hook)(MemberDescriptor *, MemberDescriptor *);

private:
    std::byte pad_0028[0x8];

public:
    const std::string &category;
    const ClassDescriptor &owner;

protected:
    virtual ~MemberDescriptor() {
    }
};
