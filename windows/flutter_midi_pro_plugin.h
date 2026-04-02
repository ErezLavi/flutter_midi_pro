#ifndef FLUTTER_PLUGIN_FLUTTER_MIDI_PRO_PLUGIN_H_
#define FLUTTER_PLUGIN_FLUTTER_MIDI_PRO_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace flutter_midi_pro {

class FlutterMidiProPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FlutterMidiProPlugin();

  virtual ~FlutterMidiProPlugin();

  // Disallow copy and assign.
  FlutterMidiProPlugin(const FlutterMidiProPlugin&) = delete;
  FlutterMidiProPlugin& operator=(const FlutterMidiProPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

}  // namespace flutter_midi_pro

#endif  // FLUTTER_PLUGIN_FLUTTER_MIDI_PRO_PLUGIN_H_
