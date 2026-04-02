#include "include/flutter_midi_pro/flutter_midi_pro_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "flutter_midi_pro_plugin.h"

void FlutterMidiProPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  flutter_midi_pro::FlutterMidiProPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
