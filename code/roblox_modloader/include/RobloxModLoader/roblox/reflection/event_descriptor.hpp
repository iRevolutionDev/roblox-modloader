#pragma once
class Event;

class EventDescriptor : public MemberDescriptor {
public:
    typedef Event ConstMember;
    typedef Event Member;
};
