#pragma once
#include "member.hpp"

#include "descriptor.hpp"
#include "property_descriptor.hpp"
#include "event_descriptor.hpp"
#include "function_descriptor.hpp"
#include "yield_function_descriptor.hpp"

class ClassDescriptor :
        public Descriptor,
        public MemberDescriptorContainer<PropertyDescriptor>,
        public MemberDescriptorContainer<EventDescriptor>,
        public MemberDescriptorContainer<FunctionDescriptor>,
        public MemberDescriptorContainer<YieldFunctionDescriptor> {
public:
    const ClassDescriptor &get_descriptor() const {
        return *this;
    }

    PropertyDescriptor *find_property_descriptor(const char *name) const {
        return MemberDescriptorContainer<PropertyDescriptor>::find_descriptor(name);
    }

    FunctionDescriptor *find_function_descriptor(const char *name) const {
        return MemberDescriptorContainer<FunctionDescriptor>::find_descriptor(name);
    }

    YieldFunctionDescriptor *find_yield_function_descriptor(const char *name) const {
        return MemberDescriptorContainer<YieldFunctionDescriptor>::find_descriptor(name);
    }

    EventDescriptor *find_event_descriptor(const char *name) const {
        return MemberDescriptorContainer<EventDescriptor>::find_descriptor(name);
    }

    PropertyDescriptor *find_property_descriptor(const char *name) {
        return get_descriptor().MemberDescriptorContainer<PropertyDescriptor>::find_descriptor(name);
    }

    FunctionDescriptor *find_function_descriptor(const char *name) {
        return get_descriptor().MemberDescriptorContainer<FunctionDescriptor>::find_descriptor(name);
    }
};
