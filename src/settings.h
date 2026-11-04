#pragma once

#include "window.h"
#include <simdjson.h>
#include <filesystem>

struct EngineSettingData {

  WindowExtent window_extent = {};
  bool isFullWindowed = false;
  bool isWindowed = false;

};


class Setting {

  EngineSettingData m_engine_setting_data {};
  simdjson::dom::parser m_parser{};

public:

  Setting(const Setting& rhs) = delete;
  Setting& operator=(const Setting& rhs) = delete;
  Setting(const Setting&& rhs ) = delete;
  Setting&& operator=(const Setting&& rhs) = delete;

  Setting() = default;
  void init();

  static Setting& get_instance() {
    static Setting* setting = new Setting();
    return *setting;
  }

  static void default_setting();


  EngineSettingData get_engine_setting_data() const { return m_engine_setting_data; };

};

class SettingUI {


public:

  static void Draw(Window& window);


};