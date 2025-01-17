#include "stdafx.hpp"

namespace ui {
using namespace std;

auto vector_getter_skin = [](void* vec, const int32_t idx, const char** out_text) {
  const auto& v = *static_cast<vector<skin::SkinInfo>*>(vec);
  if(idx < 0 || idx > static_cast<int32_t>(v.size())) {
    return false;
  }
  *out_text = idx == 0 ? "Default" : v.at(idx - 1).skinName.data();
  return true;
};

auto vector_getter_gear = [](void* vec, const int32_t idx, const char** out_text) noexcept {
  const auto& v = *static_cast<vector<const char*>*>(vec);
  if(idx < 0 || idx > static_cast<int32_t>(v.size())) {
    return false;
  }
  *out_text = v[idx];
  return true;
};

void Update() {
  using namespace config;
  const auto self = script::self;
  static int gear {};
  if(ImGui::IsKeyPressed(menuKey)) {
    showMenu = !showMenu;
    if(!showMenu) {
      Save();
    }
  }
  if(!showMenu) {
    return;
  }
  ImGui::Begin(
    "Settings", nullptr,
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar
      | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize
  );
  if(ImGui::BeginTabBar(
       "TabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTooltip
     )) {
    if(ImGui::BeginTabItem("Script")) {
      ImGui::Text("Drawing Setting:");
      ImGui::Checkbox("Show Attack Range", &config::showAttackRange);
      ImGui::Separator();
      ImGui::Text("Targeting Setting:");
      ImGui::PushItemWidth(150);
      ImGui::Combo("Current Method", &targeting, TargetingStr.data(), (int)TargetingStr.size());
      ImGui::PopItemWidth();
      ImGui::Separator();
      ImGui::Text("Key Setting:");
      ImGui::HotKey("Kite Key", kiteKey);
      ImGui::HotKey("Clean Lane Key", cleanKey);
      ImGui::EndTabItem();
    }
    if(ImGui::BeginTabItem("Skin")) {
      ImGui::Text("Skin Setting:");
      auto& values {skin::championsSkins[FNV(self->dataStack()->baseSkin.model.str)]};
      if(ImGui::Combo(
           "Current Skin", &config::currentSkin, vector_getter_skin, (void*)&values, (int)values.size() + 1
         )) {
        if(config::currentSkin > 0) {
          self->ChangeSkin(values[config::currentSkin - 1].modelName, values[config::currentSkin - 1].skinId);
        }
      }
      const auto playerHash {FNV(self->dataStack()->baseSkin.model.str)};
      if(const auto it {ranges::find_if(
           skin::specialSkins,
           [playerHash](const skin::SpecialSkin& x) {
             auto skin = script::self->dataStack()->baseSkin.skin;
             return x.champHash == playerHash && (x.skinIdStart <= skin && x.skinIdEnd >= skin);
           }
         )};
         it != skin::specialSkins.end()) {
        const auto stack {self->dataStack()};
        gear = stack->baseSkin.gear;

        if(ImGui::Combo("Current Gear", &gear, vector_getter_gear, (void*)&it->gears, (int)it->gears.size())) {
          self->dataStack()->baseSkin.gear = static_cast<int8_t>(gear);
          self->dataStack()->update(true);
        }
      }
      ImGui::Separator();
      ImGui::Text("Key Setting:");
      ImGui::HotKey("Previous Skin Key", prevSkinKey);
      ImGui::HotKey("Next Skin Key", nextSkinKey);
      ImGui::EndTabItem();
    }

    if(ImGui::BeginTabItem("Extras")) {
      ImGui::HotKey("Menu Key", menuKey);
      ImGui::Separator();
      if(ImGui::Button("Force Close")) {
        ExitProcess(0u);
      }
      ImGui::EndTabItem();
    }
    ImGui::End();
  }
}
}  // namespace ui

namespace ImGui {
bool SetToPressedKey(ImGuiKey& key) {
  for(int i = ImGuiKey::ImGuiKey_Tab; i < ImGuiKey::ImGuiKey_KeypadEqual + 1; ++i) {
    if(ImGui::IsKeyPressed(ImGuiKey(i))) {
      if(i == ImGuiKey_Escape) {
        key = ImGuiKey_None;
      } else {
        key = ImGuiKey(i);
      }
      return true;
    }
  }
  return false;
}

const char* ToString(ImGuiKey key) {
  if(key == ImGuiKey_None) {
    return "NONE";
  } else {
    return keyMap[key - ImGuiKey_Tab].data();
  }
}

void HotKey(const char* label, ImGuiKey& key, const ImVec2& size) {
  static std::map<ImGuiID, bool> activeMap;
  const auto id = GetID(label);
  PushItemWidth(150);
  LabelText(("##"s + label).data(), label);
  PopItemWidth();
  SameLine();
  if(activeMap[id]) {
    PushStyleColor(ImGuiCol_Button, GetColorU32(ImGuiCol_ButtonActive));
    Button("...", size);
    PopStyleColor();
    if((!IsItemHovered() && GetIO().MouseClicked[0]) || SetToPressedKey(key)) {
      activeMap[id] = false;
    }
  } else if(Button(ToString(key), size)) {
    activeMap[id] = true;
  }
}
}  // namespace ImGui