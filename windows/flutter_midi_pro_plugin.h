#ifndef FLUTTER_PLUGIN_FLUTTER_MIDI_PRO_PLUGIN_H_
#define FLUTTER_PLUGIN_FLUTTER_MIDI_PRO_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <fluidsynth.h>

#include <map>
#include <memory>
#include <optional>
#include <string>

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

  bool InitializeSynth();
  void ShutdownSynth();

  std::optional<int> GetIntArg(
      const flutter::EncodableMap* args,
      const std::string& key) const;

  std::optional<std::string> GetStringArg(
      const flutter::EncodableMap* args,
      const std::string& key) const;

  bool HasSoundfontId(int sfid) const;

 private:
  fluid_settings_t* settings_ = nullptr;
  fluid_synth_t* synth_ = nullptr;
  fluid_audio_driver_t* audio_driver_ = nullptr;

  // Track loaded soundfonts returned by fluid_synth_sfload.
  std::map<int, std::string> loaded_soundfonts_;
};

}  // namespace flutter_midi_pro

#endif  // FLUTTER_PLUGIN_FLUTTER_MIDI_PRO_PLUGIN_H_
