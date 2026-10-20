#include "magister_school.h"
#include "esphome/core/log.h"

namespace esphome::magister_school {

using tessera::Font;
namespace ui = tessera::ui;
namespace theme = tessera::theme;

static const char *const TAG = "magister_school";

// Helper to get next upcoming lesson from afspraken array
std::string get_next_lesson_vak(JsonObjectConst afspraken) {
  if (!afspraken) return "";
  
  uint32_t now_ts = time(nullptr);
  char now_str[32];
  strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", localtime(&now_ts));
  
  // Find first appointment that starts after now
  for (auto item : afspraken) {
    JsonObjectConst afspraak = item.value().to<JsonObjectConst>();
    if (!afspraak) continue;
    
    const char *start_str = afspraak.get<const char *>("start");
    if (!start_str) continue;
    
    // Compare strings (works for ISO format YYYY-MM-DD HH:MM:SS)
    if (strcmp(start_str, now_str) > 0) {
      const char *vak = afspraak.get<const char *>("vak");
      return vak ? vak : "";
    }
  }
  return "";
}

std::string get_next_lesson_lokaal(JsonObjectConst afspraken) {
  if (!afspraken) return "";
  
  uint32_t now_ts = time(nullptr);
  char now_str[32];
  strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", localtime(&now_ts));
  
  for (auto item : afspraken) {
    JsonObjectConst afspraak = item.value().to<JsonObjectConst>();
    if (!afspraak) continue;
    
    const char *start_str = afspraak.get<const char *>("start");
    if (!start_str) continue;
    
    if (strcmp(start_str, now_str) > 0) {
      const char *lokaal = afspraak.get<const char *>("lokaal");
      if (lokaal) {
        // Take first part before comma
        char lokaal_first[32];
        strncpy(lokaal_first, lokaal, sizeof(lokaal_first) - 1);
        lokaal_first[sizeof(lokaal_first) - 1] = '\0';
        char *comma = strchr(lokaal_first, ',');
        if (comma) *comma = '\0';
        return lokaal_first;
      }
      return "";
    }
  }
  return "";
}

uint32_t get_next_lesson_start(JsonObjectConst afspraken) {
  if (!afspraken) return 0;
  
  uint32_t now_ts = time(nullptr);
  char now_str[32];
  strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", localtime(&now_ts));
  
  for (auto item : afspraken) {
    JsonObjectConst afspraak = item.value().to<JsonObjectConst>();
    if (!afspraak) continue;
    
    const char *start_str = afspraak.get<const char *>("start");
    if (!start_str) continue;
    
    if (strcmp(start_str, now_str) > 0) {
      // Parse datetime string to timestamp
      // Format: YYYY-MM-DD HH:MM:SS
      struct tm tm = {};
      sscanf(start_str, "%d-%d-%d %d:%d:%d", 
             &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
             &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
      tm.tm_year -= 1900;
      tm.tm_mon -= 1;
      return mktime(&tm);
    }
  }
  return 0;
}

// ============== Next Lesson Tile ==============
class NextLessonTile : public tessera::Tile {
 public:
  explicit NextLessonTile(const tessera::Plugin *plugin) : plugin_(plugin) {}
  
  void create(const tessera::TileContext &c) override {
    ESP_LOGD(TAG, "NextLessonTile::create");
    parent_ = c.parent;
    
    title_ = ui::label(parent_, Font::TITLE, theme::INK);
    lv_label_set_text(title_, plugin_->text("loading"));
    lv_obj_align(title_, LV_ALIGN_TOP_LEFT, 0, 0);
    
    value_ = ui::label(parent_, Font::BODY, theme::MUTED);
    lv_label_set_text(value_, "");
    lv_obj_align(value_, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    
    icon_ = lv_obj_create(parent_);
    lv_obj_set_size(icon_, 40, 40);
    lv_obj_align(icon_, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_color(icon_, ui::color(theme::ACCENT), 0);
    lv_obj_set_style_radius(icon_, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_flag(icon_, LV_OBJ_FLAG_HIDDEN);
  }
  
  void on_state(JsonObjectConst data) override {
    ESP_LOGD(TAG, "NextLessonTile::on_state");
    
    JsonObjectConst afspraken = data["afspraken"].to<JsonObjectConst>();
    
    std::string vak = get_next_lesson_vak(afspraken);
    std::string lokaal = get_next_lesson_lokaal(afspraken);
    uint32_t start_time = get_next_lesson_start(afspraken);
    
    vak_ = vak;
    lokaal_ = lokaal;
    start_time_ = start_time;
    
    if (!vak_.empty()) {
      ui::set_text(title_, vak_.c_str());
      lv_obj_add_flag(icon_, LV_OBJ_FLAG_HIDDEN);
    } else {
      ui::set_text(title_, plugin_->text("no_lessons"));
      lv_obj_clear_flag(icon_, LV_OBJ_FLAG_HIDDEN);
    }
    
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
  uint32_t start_time_ = 0;
};

// ============== Schedule Tile ==============
class ScheduleTile : public tessera::Tile {
 public:
  explicit ScheduleTile(const tessera::Plugin *plugin) : plugin_(plugin) {}
  
  void create(const tessera::TileContext &c) override {
    ESP_LOGD(TAG, "ScheduleTile::create");
    parent_ = c.parent;
    
    title_ = ui::label(parent_, Font::TITLE, theme::INK);
    ui::set_text(title_, plugin_->text("today"));
    lv_obj_align(title_, LV_ALIGN_TOP_LEFT, 0, 0);
    
    count_ = ui::label(parent_, Font::BODY, theme::MUTED);
    ui::set_text(count_, plugin_->text("loading"));
    lv_obj_align(count_, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  }
  
  void on_state(JsonObjectConst data) override {
    ESP_LOGD(TAG, "ScheduleTile::on_state");
    
    int aantal = data["aantal_afspraken_vandaag"] | 0;
    
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
    
    // Try aantal_huiswerk_onafgerond first, then aantal_huiswerk
    int aantal = data["aantal_huiswerk_onafgerond"] | 0;
    if (aantal == 0) {
      aantal = data["aantal_huiswerk"] | 0;
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
    
    // For now, just show no grades - can be enhanced later
    JsonArrayConst cijfers = data["cijfers"].to<JsonArrayConst>();
    if (cijfers && cijfers.size() > 0) {
      // Would parse cijfers here
      ui::set_text(value_, "See app");
    } else {
      ui::set_text(value_, plugin_->text("no_grades"));
    }
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
  
  this->add_tile("next_lesson", [this]() {
    return new NextLessonTile(this);
  });
  
  this->add_tile("schedule", [this]() {
    return new ScheduleTile(this);
  });
  
  this->add_tile("homework", [this]() {
    return new HomeworkTile(this);
  });
  
  this->add_tile("grades", [this]() {
    return new GradesTile(this);
  });
  
  this->add_card("schedule_full", [this]() { return nullptr; }, false);
  this->add_card("homework_full", [this]() { return nullptr; }, false);
  this->add_card("grades_full", [this]() { return nullptr; }, false);
  
  this->add_tap_action("schedule_full", [this](const tessera::TapContext &) {});
  this->add_tap_action("homework_full", [this](const tessera::TapContext &) {});
  this->add_tap_action("grades_full", [this](const tessera::TapContext &) {});
}

}  // namespace esphome::magister_school
