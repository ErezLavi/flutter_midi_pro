#include "flutter_midi_pro_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>
#include <iostream>

namespace flutter_midi_pro {

// static
void FlutterMidiProPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "flutter_midi_pro",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<FlutterMidiProPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

FlutterMidiProPlugin::FlutterMidiProPlugin() {}

FlutterMidiProPlugin::~FlutterMidiProPlugin() {}

void FlutterMidiProPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  std::string method = method_call.method_name();

  std::cout << "[FlutterMidiPro] Method called: " << method << std::endl;

  const auto* args =
      std::get_if<flutter::EncodableMap>(method_call.arguments());

  // ---- LOAD SOUNDFONT ----
  if (method == "loadSoundfont") {
    std::cout << "loadSoundfont triggered" << std::endl;

    if (args) {
      auto it = args->find(flutter::EncodableValue("path"));
      if (it != args->end()) {
        std::string path = std::get<std::string>(it->second);
        std::cout << "Path: " << path << std::endl;
      }
    }
    result->Success(flutter::EncodableValue(1)); // dummy sfId
    return;
  }

  // ---- PLAY NOTE ----
  if (method == "playNote") {
    std::cout << "playNote triggered" << std::endl;

    if (args) {
      int note = std::get<int>(
          args->at(flutter::EncodableValue("note")));
      int velocity = std::get<int>(
          args->at(flutter::EncodableValue("velocity")));

      std::cout << "Note: " << note
                << " Velocity: " << velocity << std::endl;
    }
    result->Success();
    return;
  }

  // ---- STOP NOTE ----
  if (method == "stopNote") {
    std::cout << "stopNote triggered" << std::endl;

    if (args) {
      int note = std::get<int>(
          args->at(flutter::EncodableValue("note")));

      std::cout << "Stop note: " << note << std::endl;
    }
    result->Success();
    return;
  }

  // ---- CONTROL CHANGE ----
  if (method == "controlChange") {
    std::cout << "controlChange triggered" << std::endl;

    if (args) {
      int controller = std::get<int>(
          args->at(flutter::EncodableValue("controller")));
      int value = std::get<int>(
          args->at(flutter::EncodableValue("value")));

      std::cout << "CC: " << controller
                << " Value: " << value << std::endl;
    }
    result->Success();
    return;
  }
  // ---- SET SUSTAIN ----
  if (method == "setSustain") {
  std::cout << "setSustain triggered" << std::endl;

  if (args) {
    bool enabled = std::get<bool>(
        args->at(flutter::EncodableValue("enabled")));

    std::cout << "Sustain: " << (enabled ? "ON" : "OFF") << std::endl;
  }

  result->Success();
  return;
}

  // ---- STOP ALL NOTES ----
  if (method == "stopAllNotes") {
    std::cout << "stopAllNotes triggered" << std::endl;
    result->Success();
    return;
  }

  // ---- DISPOSE ----
  if (method == "dispose") {
    std::cout << "dispose triggered" << std::endl;
    result->Success();
    return;
  }

  result->NotImplemented();
}

}  // namespace flutter_midi_pro
