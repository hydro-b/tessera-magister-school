#pragma once
#include "esphome/core/component.h"
#include "esphome/components/smart_display/plugin_api.h"

namespace esphome::magister_school {

// Forward declarations
class NextLessonTile;
class ScheduleTile;
class HomeworkTile;
class GradesTile;

class MagisterSchool : public Component, public tessera::Plugin {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
};

}  // namespace esphome::magister_school
