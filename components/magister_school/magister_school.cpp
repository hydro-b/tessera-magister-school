#include "magister_school.h"
#include "esphome/core/log.h"

namespace esphome::magister_school {

using tessera::Font;
namespace ui = tessera::ui;

static const char *const TAG = "magister_school";

// ============== Next Lesson Tile ==============
class NextLessonTile : public tessera::Tile {
 public:
  explicit NextLessonTile(const tessera::Plugin *plugin) : plugin_(plugin) {}
  
  void create(const tessera::TileContext &c) override {
    ESP_LOGD(TAG, "NextLessonTile::create");
    parent_ = c.parent;
    
    // Title label
    title_ = ui::label(parent_, Font::TITLE, theme::INK);
    lv_label_set_text(title_, plugin_->text("loading"));
    lv_obj_align(title_, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // Value label (time and room)
    value_ = ui::label(parent_, Font::BODY, theme::MUTED);
    lv_label_set_text(value_, "");
    lv_obj_align(value_, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    
    // Icon circle
    icon_ = lv_obj_create(parent_);
    lv_obj_set_size(icon_, 40, 40);
    lv_obj_align(icon_, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_color(icon_, ui::color(theme::ACCENT), 0);
    lv_obj_set_style_radius(icon_, LV_RADIUS_CIRCLE, 0);
    
    lv_obj_add_flag(icon_, LV_OBJ_FLAG_HIDDEN);
  }
  
  void on_state(JsonObjectConst data) override {
    ESP_LOGD(TAG, "NextLessonTile::on_state");
    
    // Get entity attributes
    const char *vak = nullptr;
    const char *lokaal = nullptr;
    int64_t start_time = 0;
    
    if (data.containsKey("vak")) {
      vak = data["vak"];
    }
    if (data.containsKey("lokaal")) {
      lokaal = data["lokaal"];
    }
    if (data.containsKey("start_time")) {
      start_time = data["start_time"];
    }
    
    // Store for on_tick
    vak_ = vak ? vak : "";
    lokaal_ = lokaal ? lokaal : "";
    start_time_ = start_time;
    
    // Update title
    if (!vak_.empty()) {
      ui::set_text(title_, vak_.c_str());
      lv_obj_add_flag(icon_, LV_OBJ_FLAG_HIDDEN);
    } else {
      ui::set_text(title_, plugin_->text("no_lessons"));
      lv_obj_clear_flag(icon_, LV_OBJ_FLAG_HIDDEN);
    }
    
    // Update value
    std::string value_text;
    if (!lokaal_.empty()) {
      value_text = std::string(plugin_->text("in_room")) + " " + lokaal_;
    }
    ui::set_text(value_, value_text.empty() ? "" : value_text.c_str());
  }
  
  void on_tick(uint32_t epoch) override {
    if (start_time_ > 0 && !vak_.empty()) {
      int64_t diff = start_time_ - epoch;
      if (diff > 0 && diff < 3600) {
        // Less than an hour away
        char buf[32];
        snprintf(buf, sizeof(buf), "%d min", (int)(diff / 60));
        ui::set_text(value_, buf);
      } else if (diff <= 0) {
        ui::set_text(value_, plugin_->text("now"));
      }
    }
  }
  
  void on_theme() override {
    ui::set_color(title_, theme::INK);
    ui::set_color(value_, theme::MUTED);
    lv_obj_set_style_bg_color(icon_, ui::color(theme::ACCENT), 0);
  }
  
 private:
  const tessera::Plugin *plugin_;
  lv_obj_t *parent_;
  lv_obj_t *title_;
  lv_obj_t *value_;
  lv_obj_t *icon_;
  std::string vak_;
  std::string lokaal_;
  int64_t start_time_ = 0;
};

// ============== Schedule Tile ==============
class ScheduleTile : public tessera::Tile {
 public:
  explicit ScheduleTile(const tessera::Plugin *plugin) : plugin_(plugin) {}
  
  void create(const tessera::TileContext &c) override {
    ESP_LOGD(TAG, "ScheduleTile::create");
    parent_ = c.parent;
    
    // Title
    title_ = ui::label(parent_, Font::TITLE, theme::INK);
    ui::set_text(title_, plugin_->text("today"));
    lv_obj_align(title_, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // Count
    count_ = ui::label(parent_, Font::BODY, theme::MUTED);
    ui::set_text(count_, plugin_->text("loading"));
    lv_obj_align(count_, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  }
  
  void on_state(JsonObjectConst data) override {
    ESP_LOGD(TAG, "ScheduleTile::on_state");
    
    int aantal = 0;
    if (data.containsKey("aantal_afspraken")) {
      aantal = data["aantal_afspraken"];
    }
    
    if (aantal > 0) {
      char buf[32];
      snprintf(buf, sizeof(buf), "%d lessons today", aantal);
      ui::set_text(count_, buf);
    } else {
      ui::set_text(count_, plugin_->text("no_lessons"));
    }
  }
  
  void on_theme() override {
    ui::set_color(title_, theme::INK);
    ui::set_color(count_, theme::MUTED);
  }
  
 private:
  const tessera::Plugin *plugin_;
  lv_obj_t *parent_;
  lv_obj_t *title_;
  lv_obj_t *count_;
};

// ============== Homework Tile ==============
class HomeworkTile : public tessera::Tile {
 public:
  explicit HomeworkTile(const tessera::Plugin *plugin) : plugin_(plugin) {}
  
  void create(const tessera::TileContext &c) override {
    ESP_LOGD(TAG, "HomeworkTile::create");
    parent_ = c.parent;
    
    title_ = ui::label(parent_, Font::TITLE, theme::INK);
    ui::set_text(title_, plugin_->text("homework"));
    lv_obj_align(title_, LV_ALIGN_TOP_LEFT, 0, 0);
    
    count_ = ui::label(parent_, Font::BODY, theme::MUTED);
    ui::set_text(count_, plugin_->text("loading"));
    lv_obj_align(count_, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  }
  
  void on_state(JsonObjectConst data) override {
    ESP_LOGD(TAG, "HomeworkTile::on_state");
    
    int aantal = 0;
    if (data.containsKey("aantal_huiswerk")) {
      aantal = data["aantal_huiswerk"];
    }
    
    if (aantal > 0) {
      char buf[32];
      snprintf(buf, sizeof(buf), "%d open", aantal);
      ui::set_text(count_, buf);
    } else {
      ui::set_text(count_, plugin_->text("no_homework"));
    }
  }
  
  void on_theme() override {
    ui::set_color(title_, theme::INK);
    ui::set_color(count_, theme::MUTED);
  }
  
 private:
  const tessera::Plugin *plugin_;
  lv_obj_t *parent_;
  lv_obj_t *title_;
  lv_obj_t *count_;
};

// ============== Grades Tile ==============
class GradesTile : public tessera::Tile {
 public:
  explicit GradesTile(const tessera::Plugin *plugin) : plugin_(plugin) {}
  
  void create(const tessera::TileContext &c) override {
    ESP_LOGD(TAG, "GradesTile::create");
    parent_ = c.parent;
    
    title_ = ui::label(parent_, Font::TITLE, theme::INK);
    ui::set_text(title_, plugin_->text("grades"));
    lv_obj_align(title_, LV_ALIGN_TOP_LEFT, 0, 0);
    
    value_ = ui::label(parent_, Font::BODY, theme::MUTED);
    ui::set_text(value_, plugin_->text("loading"));
    lv_obj_align(value_, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  }
  
  void on_state(JsonObjectConst data) override {
    ESP_LOGD(TAG, "GradesTile::on_state");
    
    // Would extract recent grade or average from cijfers array
    ui::set_text(value_, plugin_->text("no_grades"));
  }
  
  void on_theme() override {
    ui::set_color(title_, theme::INK);
    ui::set_color(value_, theme::MUTED);
  }
  
 private:
  const tessera::Plugin *plugin_;
  lv_obj_t *parent_;
  lv_obj_t *title_;
  lv_obj_t *value_;
};

// ============== Plugin ==============
void MagisterSchool::setup() {
  ESP_LOGI(TAG, "Setting up Magister School plugin");
  
  // Next lesson tile
  this->add_tile("next_lesson", [this]() {
    return new NextLessonTile(this);
  });
  
  // Schedule tile
  this->add_tile("schedule", [this]() {
    return new ScheduleTile(this);
  });
  
  // Homework tile
  this->add_tile("homework", [this]() {
    return new HomeworkTile(this);
  });
  
  // Grades tile
  this->add_tile("grades", [this]() {
    return new GradesTile(this);
  });
  
  // Cards (placeholder - not yet implemented)
  this->add_card("schedule_full", [this]() { return nullptr; }, false);
  this->add_card("homework_full", [this]() { return nullptr; }, false);
  this->add_card("grades_full", [this]() { return nullptr; }, false);
  
  // Tap actions (placeholder - not yet implemented)
  this->add_tap_action("schedule_full", [this](const tessera::TapContext &) {});
  this->add_tap_action("homework_full", [this](const tessera::TapContext &) {});
  this->add_tap_action("grades_full", [this](const tessera::TapContext &) {});
}

}  // namespace esphome::magister_school
