#include "settings.h"

#include "imgui.h"
#include "render.h"

void Setting::init() {

  auto path =  std::filesystem::current_path().parent_path() / "engine_settings.json";

  auto elements =  m_parser.load(path.string());
  if (elements.error() != simdjson::SUCCESS) {
    std::cerr << "Engine Setting file was not found: Setting will be by default" << std::endl;
  }

  else {
      std::cerr << "Engine setting loaded: " << path.string() << std::endl;
  }

  const auto& object = elements.get_object();

  m_engine_setting_data.window_extent.m_window_height = static_cast<uint32_t>(object["window_resolution"]["height"].get_uint64());
  m_engine_setting_data.window_extent.m_window_width = static_cast<uint32_t>(object["window_resolution"]["width"].get_uint64());

  m_engine_setting_data.isFullWindowed = object["isFullWindowed"].get_bool();
  m_engine_setting_data.isWindowed = object["isWindowed"].get_bool();



}

void Setting::default_setting() {

  EngineSettingData data{};
  data.isFullWindowed = false;
  data.isWindowed = true;
  data.window_extent.m_window_width = 1024;
  data.window_extent.m_window_height = 720;


}



void SettingUI::Draw(Window& window) {
  bool bOpen;
  ImGui::Begin("Engine Settings:", &bOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoResize);
  ImGui::SetWindowSize(ImVec2(350, 350));

  const char* array[] = {"800/600", "1024/720", "1920/1080", "2560/1440" };

  constexpr WindowExtent array_extents[] = {
    {.m_window_width = 800, .m_window_height = 600}, { .m_window_width = 1024, .m_window_height = 720},
{.m_window_width = 1920, .m_window_height = 1080}, { .m_window_width = 2560, .m_window_height = 1440}
  };


  if (ImGui::BeginCombo("Window Resolution: ", nullptr)) {
    bool blsSelected = false;
    for (int i = 0; i < IM_ARRAYSIZE(array); ++i) {

      if (ImGui::Selectable(array[i], blsSelected)) {

      blsSelected = array[i];
      if (blsSelected == true) {
          window.set_window_extent(array_extents[i].m_window_width ,array_extents[i].m_window_height);
          Render& render = Render::getInstance();

          render.resize();

      }

      }


    }

  ImGui::EndCombo();

  }


  ImGui::End();



}


