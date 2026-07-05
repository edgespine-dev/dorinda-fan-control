#pragma once

#include <stddef.h>
#include <stdint.h>

namespace cal {

constexpr size_t MAX_ROWS = 16;

struct Row
{
    uint16_t pwm;
    uint16_t current_ma;
};

bool init();
bool load();
bool save();
bool upsert(uint16_t pwm, uint16_t current_ma);
size_t count();
const Row* rows();
int interpolate_ma(uint16_t pwm);  // -1 if matrix empty
void dump_serial();
void clear();

}  // namespace cal
