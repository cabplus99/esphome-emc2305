#include "emc2305_output.h"

namespace esphome {
namespace emc2305 {

void EMC2305Output::write_state(float state) { this->parent_->set_duty_cycle(state); }

}  // namespace emc2305
}  // namespace esphome
