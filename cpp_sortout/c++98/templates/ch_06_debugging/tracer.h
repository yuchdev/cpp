#pragma once

/**@brief
The class for passing as a template argument,
perform primary checks, and tracing the efficiency of the algorithm
*/
class tracer
{
public:

    tracer(int v = 0);
    ~tracer();

    // Accessors
    static long creations() { return s_created; }
    static long copies() { return s_copied; }
    static long assigmennts() { return s_assigned; }
    static long comparasions() { return s_compared; }
    static long max_objects() { return s_max_objects; }

    // Trace methods
    tracer(const tracer& t);
    tracer& operator=(const tracer& t);
    bool operator<(const tracer& t) const;
    bool operator>(const tracer& t) const;
    void trace_msg_create() const;
    void trace_msg_destroy() const;
    void trace_msg_assign() const;
    void trace_msg_compare() const;

protected:

    // Max number of objects used
    static inline void update_max_objects();
private:
    // 
    int _val;

    // Generation of tracer (how many times has been copied)
    int _generation;

    // Create count
    static long s_objects;

    // Create count
    static long s_created;

    // Copy count
    static long s_copied;

    // Assignment count
    static long s_assigned;

    // Copmarison count
    static long s_compared;

    // Max object created
    static long s_max_objects;
};
