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
    using PropertyDescriptors = MemberDescriptorContainer<PropertyDescriptor>::Collection;
    using FunctionDescriptors = MemberDescriptorContainer<FunctionDescriptor>::Collection;
    using YieldFunctionDescriptors = MemberDescriptorContainer<YieldFunctionDescriptor>::Collection;
    using EventDescriptors = MemberDescriptorContainer<EventDescriptor>::Collection;

    const ClassDescriptor &get_descriptor() const {
        return *this;
    }

    const PropertyDescriptors &property_descriptors() const {
        return MemberDescriptorContainer<PropertyDescriptor>::get_descriptors();
    }

    const FunctionDescriptors &function_descriptors() const {
        return MemberDescriptorContainer<FunctionDescriptor>::get_descriptors();
    }

    const YieldFunctionDescriptors &yield_function_descriptors() const {
        return MemberDescriptorContainer<YieldFunctionDescriptor>::get_descriptors();
    }

    const EventDescriptors &event_descriptors() const {
        return MemberDescriptorContainer<EventDescriptor>::get_descriptors();
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
