#include "magister_school.h"
#include "esphome/core/log.h"
#include <ctime>

namespace esphome::magister_school {

using tessera::Font;
namespace ui = tessera::ui;

static const char *const TAG = "magister_school";

// Helper to get next upcoming lesson from afspraken array
std::string get_next_lesson_vak(JsonObjectConst afspraken) {
  if (!afspraken) return "";
  
  time_t now_ts = time(nullptr);
  char now_str[32];
  strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", localtime(&now_ts));
  
  for (auto item : afspraken) {
    JsonObjectConst afspraak = item.value();
    if (!afspraak) continue;
    
    const char *start_str = afspraak["start"];
    if (!start_str) continue;
    
    if (strcmp(start_str, now_str) > 0) {
      const char *vak = afspraak["vak"];
      return vak ? vak : "";
    }
  }
  return "";
}

std::string get_next_lesson_lokaal(JsonObjectConst afspraken) {
  if (!afspraken) return "";
  
  time_t now_ts = time(nullptr);
  char now_str[32];
  strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", localtime(&now_ts));
  
  for (auto item : afspraken) {
    JsonObjectConst afspraak = item.value();
    if (!afspraak) continue;
    
    const char *start_str = afspraak["start"];
    if (!start_str) continue;
    
    if (strcmp(start_str, now_str) > 0) {
      const char *lokaal = afspraak["lokaal"];
      if (lokaal) {
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

time_t get_next_lesson_start(JsonObjectConst afspraken) {
  if (!afspraken) return 0;
  
  time_t now_ts = time(nullptr);
  char now_str[32];
  strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", localtime(&now_ts));
  
  for (auto item : afspraken) {
    JsonObjectConst afspraak = item.value();
    if (!afspraak) continue;
    
    const char *start_str = afspraak["start"];
    if (!start_str) continue;
    
    if (strcmp(start_str, now_str) > 0) {
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
    
    JsonObjectConst attributes = data["attributes"];
    JsonObjectConst afspraken;
    
    if (attributes) {
      afspraken = attributes["afspraken"];
    }
    
    std::string vak = get_next_lesson_vak(afspraken);
    std::string lokaal = get_next_lesson_lokaal(afspraken);
    time_t start_time = get_next_lesson_start(afspraken);
    
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
  time_t start_time_ = 0;
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
    ESP_LOGI(TAG, "=== ScheduleTile on_state called ===");
    ESP_LOGI(TAG, "Data size: %d", data.size());
    
    // Log all keys in data
    for (auto item : data) {
      ESP_LOGI(TAG, "Top-level key: %s", item.key().c_str());
    }
    
    // Check for state
    const char *state = data["state"];
    ESP_LOGI(TAG, "State: %s", state ? state : "NULL");
    
    // Check for name
    const char *name = data["name"];
    ESP_LOGI(TAG, "Name: %s", name ? name : "NULL");
    
    // Get attributes
    JsonObjectConst attributes = data["attributes"];
    ESP_LOGI(TAG, "Attributes exists: %s", attributes ? "YES" : "NO");
    
    int aantal = 0;
    
    if (attributes) {
      // Log all attribute keys
      for (auto item : attributes) {
        ESP_LOGI(TAG, "Attribute key: %s", item.key().c_str());
      }
      
      // Try multiple possible attribute names
      aantal = attributes["aantal_afspraken_vandaag"] | 0;
      ESP_LOGI(TAG, "aantal_afspraken_vandaag (lowercase): %d", aantal);
      
      if (aantal == 0) {
        aantal = attributes["Aantal afspraken vandaag"] | 0;
        ESP_LOGI(TAG, "Aantal afspraken vandaag (capital): %d", aantal);
      }
      
      // Check afspraken array
      if (attributes["afspraken"].is<JsonArrayConst>()) {
        JsonArrayConst afspraken = attributes["afspraken"];
        ESP_LOGI(TAG, "Afspraken array size: %d", afspraken.size());
        
        // Count today's appointments
        time_t now_ts = time(nullptr);
        struct tm *now_tm = localtime(&now_ts);
        char today_str[11];
        strftime(today_str, sizeof(today_str), "%Y-%m-%d", now_tm);
        ESP_LOGI(TAG, "Today's date: %s", today_str);
        
        int counted = 0;
        for (auto item : afspraken) {
          JsonObjectConst afspraak = item;
          const char *start = afspraak["start"];
          if (start && strncmp(start, today_str, 10) == 0) {
            counted++;
            ESP_LOGI(TAG, "Found today's appointment: %s", start);
          }
        }
        ESP_LOGI(TAG, "Counted today's appointments: %d", counted);
        
        if (aantal == 0) {
          aantal = counted;
        }
      }
      
      ESP_LOGI(TAG, "Final aantal: %d", aantal);
    } else {
      ESP_LOGW(TAG, "No attributes in data!");
    }
    
    if (aantal > 0) {
      char buf[32];
      snprintf(buf, sizeof(buf), "%d lessons today", aantal);
      ui::set_text(count_, buf);
      ESP_LOGI(TAG, "Showing: %s", buf);
    } else {
      if (!state || strcmp(state, "unavailable") == 0 || strcmp(state, "unknown") == 0) {
        ESP_LOGW(TAG, "Entity unavailable or unknown: %s", state ? state : "NULL");
        ui::set_text(count_, "No entity");
      } else {
        ESP_LOGW(TAG, "Data received but aantal = 0, state = %s", state ? state : "null");
        ui::set_text(count_, "0 lessons");
      }
    }
    ESP_LOGI(TAG, "=== End on_state ===");
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
    
    JsonObjectConst attributes = data["attributes"];
    int aantal = 0;
    
    if (attributes) {
      aantal = attributes["aantal_huiswerk_onafgerond"] | 0;
      if (aantal == 0) {
        aantal = attributes["aantal_huiswerk"] | 0;
      }
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
    
    JsonObjectConst attributes = data["attributes"];
    JsonArrayConst cijfers;
    
    if (attributes) {
      cijfers = attributes["recente_cijfers"];
    }
    
    if (cijfers && cijfers.size() > 0) {
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
