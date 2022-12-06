#include "tracer.h"
#include <iostream>
using std::cout;
using std::endl;

long tracer::s_objects = 0;
long tracer::s_created = 0;
long tracer::s_copied = 0;
long tracer::s_assigned = 0;
long tracer::s_compared = 0;
long tracer::s_max_objects = 0;


tracer::tracer(int v/* = 0*/) : _val(v), _generation(1)
{
    ++s_created;
    ++s_objects;
    update_max_objects();
    trace_msg_create();
}

tracer::~tracer()
{
    --s_objects;
    update_max_objects();
    trace_msg_destroy();
}

tracer::tracer(const tracer& t) : _val(t._val), _generation(t._generation + 1)
{
    ++s_created;
    ++s_copied;
    ++s_objects;
    update_max_objects();
    trace_msg_create();
}

tracer& tracer::operator=(const tracer& t)
{
    if (&t != this) {
        ++s_assigned;
        _val = t._val;
        trace_msg_assign();
    }
    return *this;
}

bool tracer::operator<(const tracer& t) const
{
    ++s_compared;
    return _val < t._val;
}

bool tracer::operator>(const tracer& t) const
{
    ++s_compared;
    return !(operator<(t));
}

void tracer::trace_msg_create() const
{
    cout << "tracer #" << s_created << " created"
        << " generation # " << _generation
        << " total " << s_created << " created"
        << endl;
}

void tracer::trace_msg_destroy() const
{
    cout << "tracer generation # "
        << _generation << " destroyed"
        << " total " << s_created << " created"
        << endl;
}

void tracer::trace_msg_assign() const
{
    cout << "assignment # " << s_assigned << " assigned"
        << " generation # " << _generation
        << " total " << s_created << " created"
        << endl;
}

void tracer::trace_msg_compare() const
{
    cout << "comparasion # " << s_compared << " compared"
        << " generation # " << _generation
        << " total " << s_created << " created"
        << endl;
}

void tracer::update_max_objects()
{
    if (s_objects > s_max_objects)
        s_max_objects = s_objects;
}

