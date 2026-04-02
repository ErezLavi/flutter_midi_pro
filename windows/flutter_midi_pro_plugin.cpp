#include "include/flutter_midi_pro/flutter_midi_pro_plugin_c_api.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <fluidsynth.h>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <iostream>

namespace flutter_midi_pro {
class FlutterMidiProPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FlutterMidiProPlugin();
  virtual ~FlutterMidiProPlugin();

  FlutterMidiProPlugin(const FlutterMidiProPlugin&) = delete;
  FlutterMidiProPlugin& operator=(const FlutterMidiProPlugin&) = delete;

 public:
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
  int ResolveChannelForSoundfont(int channel, int sfid);

 private:
  fluid_settings_t* settings_ = nullptr;
  fluid_synth_t* synth_ = nullptr;
  fluid_audio_driver_t* audio_driver_ = nullptr;

  // Track loaded soundfonts returned by fluid_synth_sfload.
  std::map<int, std::string> loaded_soundfonts_;

  // Optional mapping: channel -> currently selected sfid.
  // This lets sfId stay meaningful without changing the Dart API.
  std::map<int, int> channel_to_sfid_;
};

FlutterMidiProPlugin::FlutterMidiProPlugin() {
  InitializeSynth();
}

FlutterMidiProPlugin::~FlutterMidiProPlugin() {
  ShutdownSynth();
}

void FlutterMidiProPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "flutter_midi_pro",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<FlutterMidiProPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](
          const auto& call,
          auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

bool FlutterMidiProPlugin::InitializeSynth() {
  if (synth_ != nullptr) {
    return true;
  }

  settings_ = new_fluid_settings();
  if (!settings_) {
    std::cerr << "[flutter_midi_pro] Failed to create FluidSynth settings\n";
    return false;
  }

  // Good defaults for Windows desktop use.
  fluid_settings_setnum(settings_, "synth.gain", 0.8);
  fluid_settings_setint(settings_, "synth.polyphony", 256);

  // You can force driver if needed, but default often works.
  // fluid_settings_setstr(settings_, "audio.driver", "dsound");

  synth_ = new_fluid_synth(settings_);
  if (!synth_) {
    std::cerr << "[flutter_midi_pro] Failed to create FluidSynth synth\n";
    ShutdownSynth();
    return false;
  }

  audio_driver_ = new_fluid_audio_driver(settings_, synth_);
  if (!audio_driver_) {
    std::cerr << "[flutter_midi_pro] Failed to create FluidSynth audio driver\n";
    ShutdownSynth();
    return false;
  }

  std::cout << "[flutter_midi_pro] FluidSynth initialized successfully\n";
  return true;
}

void FlutterMidiProPlugin::ShutdownSynth() {
  loaded_soundfonts_.clear();
  channel_to_sfid_.clear();

  if (audio_driver_) {
    delete_fluid_audio_driver(audio_driver_);
    audio_driver_ = nullptr;
  }

  if (synth_) {
    delete_fluid_synth(synth_);
    synth_ = nullptr;
  }

  if (settings_) {
    delete_fluid_settings(settings_);
    settings_ = nullptr;
  }
}

std::optional<int> FlutterMidiProPlugin::GetIntArg(
    const flutter::EncodableMap* args,
    const std::string& key) const {
  if (!args) return std::nullopt;

  auto it = args->find(flutter::EncodableValue(key));
  if (it == args->end()) return std::nullopt;

  if (const int* value = std::get_if<int>(&it->second)) {
    return *value;
  }

  return std::nullopt;
}

std::optional<std::string> FlutterMidiProPlugin::GetStringArg(
    const flutter::EncodableMap* args,
    const std::string& key) const {
  if (!args) return std::nullopt;

  auto it = args->find(flutter::EncodableValue(key));
  if (it == args->end()) return std::nullopt;

  if (const std::string* value = std::get_if<std::string>(&it->second)) {
    return *value;
  }

  return std::nullopt;
}

bool FlutterMidiProPlugin::HasSoundfontId(int sfid) const {
  return loaded_soundfonts_.find(sfid) != loaded_soundfonts_.end();
}

int FlutterMidiProPlugin::ResolveChannelForSoundfont(int channel, int sfid) {
  // Minimal behavior:
  // - If a valid sfId is provided, bind that sfId to the channel.
  // - FluidSynth programs are channel-based, not per-note soundfont-based.
  // - This preserves sfId semantics in a practical way for Windows.
  if (HasSoundfontId(sfid)) {
    channel_to_sfid_[channel] = sfid;
  }
  return channel;
}

void FlutterMidiProPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  const std::string method_name = method_call.method_name();
  std::cout << "[flutter_midi_pro] Method called: " << method_name << "\n";

  if (!InitializeSynth()) {
    result->Error("synth_init_failed", "FluidSynth initialization failed");
    return;
  }

  const auto* args = std::get_if<flutter::EncodableMap>(method_call.arguments());

  if (method_name == "loadSoundfont") {
    auto path_opt = GetStringArg(args, "path");
    if (!path_opt.has_value()) {
      result->Error("bad_args", "Missing soundfont path");
      return;
    }

    const std::string& path = *path_opt;
    int sfid = fluid_synth_sfload(synth_, path.c_str(), 1);

    if (sfid == FLUID_FAILED) {
      std::cerr << "[flutter_midi_pro] Failed to load soundfont: " << path << "\n";
      result->Error("sfload_failed", "Failed to load soundfont");
      return;
    }

    loaded_soundfonts_[sfid] = path;

    std::cout << "[flutter_midi_pro] Soundfont loaded: " << path
              << " -> sfid=" << sfid << "\n";

    result->Success(flutter::EncodableValue(sfid));
    return;
  }

  if (method_name == "selectInstrument") {
    auto sfid_opt = GetIntArg(args, "sfId");
    auto channel_opt = GetIntArg(args, "channel");
    auto bank_opt = GetIntArg(args, "bank");
    auto program_opt = GetIntArg(args, "program");

    if (!sfid_opt || !channel_opt || !bank_opt || !program_opt) {
      result->Error("bad_args", "Missing sfId/channel/bank/program");
      return;
    }

    int rc = fluid_synth_program_select(
        synth_,
        *channel_opt,
        *sfid_opt,
        *bank_opt,
        *program_opt);

    if (rc == FLUID_FAILED) {
      result->Error("program_select_failed", "Failed to select instrument");
      return;
    }

    std::cout << "[flutter_midi_pro] program selected "
              << "sfid=" << *sfid_opt
              << " ch=" << *channel_opt
              << " bank=" << *bank_opt
              << " prog=" << *program_opt << "\n";

    result->Success();
    return;
  }

  if (method_name == "playNote") {
    auto key_opt = GetIntArg(args, "key");
    auto velocity_opt = GetIntArg(args, "velocity");
    auto channel_opt = GetIntArg(args, "channel");
    auto sfid_opt = GetIntArg(args, "sfId");

    if (!key_opt || !velocity_opt || !channel_opt) {
      result->Error("bad_args", "Missing key/velocity/channel");
      return;
    }

    int key = *key_opt;
    int velocity = *velocity_opt;
    int channel = *channel_opt;
    int sfid = sfid_opt.value_or(-1);

    ResolveChannelForSoundfont(channel, sfid);

    int rc = fluid_synth_noteon(synth_, channel, key, velocity);
    if (rc == FLUID_FAILED) {
      result->Error("noteon_failed", "fluid_synth_noteon failed");
      return;
    }

    std::cout << "[flutter_midi_pro] noteon ch=" << channel
              << " key=" << key
              << " vel=" << velocity
              << " sfid=" << sfid << "\n";

    result->Success();
    return;
  }

  if (method_name == "stopNote") {
    auto key_opt = GetIntArg(args, "key");
    auto channel_opt = GetIntArg(args, "channel");

    if (!key_opt || !channel_opt) {
      result->Error("bad_args", "Missing key/channel");
      return;
    }

    int key = *key_opt;
    int channel = *channel_opt;

    int rc = fluid_synth_noteoff(synth_, channel, key);
    if (rc == FLUID_FAILED) {
      result->Error("noteoff_failed", "fluid_synth_noteoff failed");
      return;
    }

    std::cout << "[flutter_midi_pro] noteoff ch=" << channel
              << " key=" << key << "\n";

    result->Success();
    return;
  }

  if (method_name == "controlChange") {
    auto channel_opt = GetIntArg(args, "channel");
    auto controller_opt = GetIntArg(args, "controller");
    auto value_opt = GetIntArg(args, "value");

    if (!channel_opt || !controller_opt || !value_opt) {
      result->Error("bad_args", "Missing channel/controller/value");
      return;
    }

    int rc = fluid_synth_cc(
        synth_,
        *channel_opt,
        *controller_opt,
        *value_opt);

    if (rc == FLUID_FAILED) {
      result->Error("cc_failed", "fluid_synth_cc failed");
      return;
    }

    result->Success();
    return;
  }

  if (method_name == "setSustain") {
    auto channel_opt = GetIntArg(args, "channel");
    auto enabled_opt = GetIntArg(args, "enabled");
    
    if (!channel_opt || !enabled_opt) {
      result->Error("bad_args", "Missing channel/value");
      return;
    }

    // MIDI CC 64 = sustain pedal
    int sustain_value = (*enabled_opt != 0) ? 127 : 0;

    int rc = fluid_synth_cc(synth_, *channel_opt, 64, sustain_value);
    if (rc == FLUID_FAILED) {
      result->Error("sustain_failed", "Failed to set sustain");
      return;
    }

    result->Success();
    return;
  }

  if (method_name == "stopAllNotes") {
    // Stop notes on standard MIDI channels.
    for (int ch = 0; ch < 16; ++ch) {
      fluid_synth_all_notes_off(synth_, ch);
      fluid_synth_all_sounds_off(synth_, ch);
    }

    result->Success();
    return;
  }

  result->NotImplemented();
}

}  // namespace flutter_midi_pro