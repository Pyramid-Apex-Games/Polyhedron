#pragma once
#include <nlohmann/json_fwd.hpp>

struct SoundDef {
    std::string sound;
    int volume = 0;
    SoundDef() = default;
    SoundDef(const std::string& sound, unsigned char volume);
};

class SoundConfig {
    friend void from_json(const nlohmann::json& document, SoundConfig& soundDef);
    friend void to_json(nlohmann::json& document, const SoundConfig& entity_t);
public:
    SoundConfig();
    void Load();

private:
    std::vector<SoundDef> m_Sounds;
};

void from_json(const nlohmann::json& document, SoundDef& soundDef);
void to_json(const nlohmann::json& document, SoundDef& soundDef);

void from_json(const nlohmann::json& document, SoundConfig& soundConfig);
void to_json(const nlohmann::json& document, SoundConfig& soundConfig);