#ifndef STRESS_H
#define STRESS_H

#include <string>


enum type {
    CPU_RT,
    TIMER,
    IO
};

struct partition_id {
    int major;
    int minor;
};

class stress
{
public:
    stress(type type, int num, int duration, int intensity = -1);
    ~stress();

    static partition_id get_partition_id();
    void stop();

private:
    type m_type;
};

#endif // STRESS_H
