#pragma once
#include <memory>
#include <vector>

class ClassDescriptor; // for now, I'll do the reflection later.

class Instance : public std::enable_shared_from_this<Instance> {
public:
    void *vtable;
    ClassDescriptor *class_descriptor;

private:
    std::byte pad_0048[0x30];

public:
    Instance *parent;

private:
    std::byte pad_0058[0x20];

public:
    std::string_view name;

    // shared_ptr causes some crashes because has invalid reference count TODO: fix it later
    std::shared_ptr<std::vector<std::shared_ptr<Instance> > > children;

    template<typename T = Instance>
    T *as() {
        return static_cast<T *>(this);
    }

    std::shared_ptr<Instance> current() {
        return shared_from_this();
    }
};
