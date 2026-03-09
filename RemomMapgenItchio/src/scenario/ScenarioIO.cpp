// JSON serialization for .mom_scenario files.
// Powered by Claude.

#include "scenario/ScenarioIO.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstring>

// Base64 encode/decode for binary terrain data.
// Powered by Claude.
namespace {

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const uint8_t* data, size_t len) {
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t n = static_cast<uint32_t>(data[i]) << 16;
        if (i + 1 < len) n |= static_cast<uint32_t>(data[i + 1]) << 8;
        if (i + 2 < len) n |= static_cast<uint32_t>(data[i + 2]);
        out.push_back(b64_table[(n >> 18) & 0x3F]);
        out.push_back(b64_table[(n >> 12) & 0x3F]);
        out.push_back((i + 1 < len) ? b64_table[(n >> 6) & 0x3F] : '=');
        out.push_back((i + 2 < len) ? b64_table[n & 0x3F] : '=');
    }
    return out;
}

int b64_char_val(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

std::vector<uint8_t> base64_decode(const std::string& encoded) {
    std::vector<uint8_t> out;
    out.reserve((encoded.size() / 4) * 3);
    for (size_t i = 0; i < encoded.size(); i += 4) {
        uint32_t n = 0;
        int pad = 0;
        for (int j = 0; j < 4 && i + j < encoded.size(); ++j) {
            char c = encoded[i + j];
            if (c == '=') { ++pad; continue; }
            int v = b64_char_val(c);
            if (v < 0) continue;
            n |= static_cast<uint32_t>(v) << (18 - j * 6);
        }
        out.push_back(static_cast<uint8_t>((n >> 16) & 0xFF));
        if (pad < 2) out.push_back(static_cast<uint8_t>((n >> 8) & 0xFF));
        if (pad < 1) out.push_back(static_cast<uint8_t>(n & 0xFF));
    }
    return out;
}

// Convert Roughness enum to string. Powered by Claude.
std::string roughness_to_string(mom::Roughness r) {
    switch (r) {
        case mom::ROUGHNESS_SMOOTH: return "SMOOTH";
        case mom::ROUGHNESS_MEDIUM: return "MEDIUM";
        case mom::ROUGHNESS_ROUGH:  return "ROUGH";
        default: return "MEDIUM";
    }
}

// Convert string to Roughness enum. Powered by Claude.
mom::Roughness string_to_roughness(const std::string& s) {
    if (s == "SMOOTH") return mom::ROUGHNESS_SMOOTH;
    if (s == "ROUGH")  return mom::ROUGHNESS_ROUGH;
    return mom::ROUGHNESS_MEDIUM;
}

} // anonymous namespace

namespace mom {

using json = nlohmann::json;

// Serialize a city to JSON.
// Powered by Claude.
static json city_to_json(const City& c) {
    json j;
    j["name"] = std::string(c.name, strnlen(c.name, LEN_CITY_NAME));
    j["race"] = c.race;
    j["x"] = c.wx;
    j["y"] = c.wy;
    j["plane"] = c.wp;
    j["owner"] = c.owner_idx;
    j["population"] = c.population;
    j["size"] = c.size;

    json bldgs = json::array();
    for (int i = 0; i < NUM_BUILDINGS; ++i)
        bldgs.push_back(c.bldg_status[i]);
    j["buildings"] = bldgs;

    return j;
}

// Deserialize a city from JSON.
// Powered by Claude.
static City city_from_json(const json& j) {
    City c{};
    std::string name = j.value("name", "");
    std::strncpy(c.name, name.c_str(), LEN_CITY_NAME - 1);
    c.race      = j.value("race", (int8_t)0);
    c.wx        = j.value("x", (int8_t)0);
    c.wy        = j.value("y", (int8_t)0);
    c.wp        = j.value("plane", (int8_t)0);
    c.owner_idx = j.value("owner", (int8_t)0);
    c.population = j.value("population", (int8_t)1);
    c.size      = j.value("size", (int8_t)1);

    if (j.contains("buildings")) {
        auto& bldgs = j["buildings"];
        for (int i = 0; i < NUM_BUILDINGS && i < (int)bldgs.size(); ++i)
            c.bldg_status[i] = bldgs[i].get<int8_t>();
    }
    return c;
}

// Serialize a unit to JSON.
// Powered by Claude.
static json unit_to_json(const Unit& u) {
    json j;
    j["type"]  = u.type;
    j["x"]     = u.wx;
    j["y"]     = u.wy;
    j["plane"] = u.wp;
    j["owner"] = u.owner_idx;
    j["level"] = u.Level;
    j["xp"]    = u.XP;
    j["hero_slot"]    = u.Hero_Slot;
    j["enchantments"] = u.enchantments;
    return j;
}

// Deserialize a unit from JSON.
// Powered by Claude.
static Unit unit_from_json(const json& j) {
    Unit u{};
    u.type      = j.value("type", (uint8_t)0);
    u.wx        = j.value("x", (int8_t)0);
    u.wy        = j.value("y", (int8_t)0);
    u.wp        = j.value("plane", (int8_t)0);
    u.owner_idx = j.value("owner", (int8_t)0);
    u.Level     = j.value("level", (int8_t)1);
    u.XP        = j.value("xp", (int16_t)0);
    u.Hero_Slot = j.value("hero_slot", (int8_t)-1);
    u.enchantments = j.value("enchantments", (uint32_t)0);
    return u;
}

// Serialize a wizard to JSON (key fields only for readability).
// Powered by Claude.
static json wizard_to_json(const Wizard& w) {
    json j;
    j["id"]   = w.wizard_id;
    j["name"] = std::string(w.name, strnlen(w.name, LEN_WIZARD_NAME));
    j["banner"]      = w.banner_id;
    j["capital_race"] = w.capital_race;
    j["gold"]  = w.gold_reserve;
    j["mana"]  = w.mana_reserve;
    j["fame"]  = w.fame;
    j["tax_rate"] = w.tax_rate;

    json sr = json::array();
    for (int i = 0; i < 5; ++i) sr.push_back(w.spellranks[i]);
    j["spellranks"] = sr;

    // Retorts as object for readability
    json retorts;
    retorts["alchemy"]         = w.alchemy;
    retorts["warlord"]         = w.warlord;
    retorts["chaos_mastery"]   = w.chaos_mastery;
    retorts["nature_mastery"]  = w.nature_mastery;
    retorts["sorcery_mastery"] = w.sorcery_mastery;
    retorts["infernal_power"]  = w.infernal_power;
    retorts["divine_power"]    = w.divine_power;
    retorts["sage_master"]     = w.sage_master;
    retorts["channeler"]       = w.channeler;
    retorts["myrran"]          = w.myrran;
    retorts["archmage"]        = w.archmage;
    retorts["mana_focusing"]   = w.mana_focusing;
    retorts["node_mastery"]    = w.node_mastery;
    retorts["famous"]          = w.famous;
    retorts["runemaster"]      = w.runemaster;
    retorts["conjurer"]        = w.conjurer;
    retorts["charismatic"]     = w.charismatic;
    retorts["artificer"]       = w.artificer;
    j["retorts"] = retorts;

    // Spells list as base64
    j["spells_b64"] = base64_encode(w.spells_list, NUM_SPELLS);

    return j;
}

// Deserialize a wizard from JSON.
// Powered by Claude.
static Wizard wizard_from_json(const json& j) {
    Wizard w{};
    w.wizard_id = j.value("id", (uint8_t)0);
    std::string name = j.value("name", "");
    std::strncpy(w.name, name.c_str(), LEN_WIZARD_NAME - 1);
    w.banner_id    = j.value("banner", (uint8_t)0);
    w.capital_race = j.value("capital_race", (uint8_t)0);
    w.gold_reserve = j.value("gold", (uint16_t)100);
    w.mana_reserve = j.value("mana", (uint16_t)0);
    w.fame         = j.value("fame", (uint16_t)0);
    w.tax_rate     = j.value("tax_rate", (uint16_t)1);

    if (j.contains("spellranks")) {
        auto& sr = j["spellranks"];
        for (int i = 0; i < 5 && i < (int)sr.size(); ++i)
            w.spellranks[i] = sr[i].get<int16_t>();
    }

    if (j.contains("retorts")) {
        auto& r = j["retorts"];
        w.alchemy         = r.value("alchemy", (int8_t)0);
        w.warlord         = r.value("warlord", (int8_t)0);
        w.chaos_mastery   = r.value("chaos_mastery", (int8_t)0);
        w.nature_mastery  = r.value("nature_mastery", (int8_t)0);
        w.sorcery_mastery = r.value("sorcery_mastery", (int8_t)0);
        w.infernal_power  = r.value("infernal_power", (int8_t)0);
        w.divine_power    = r.value("divine_power", (int8_t)0);
        w.sage_master     = r.value("sage_master", (int8_t)0);
        w.channeler       = r.value("channeler", (int8_t)0);
        w.myrran          = r.value("myrran", (int8_t)0);
        w.archmage        = r.value("archmage", (int8_t)0);
        w.mana_focusing   = r.value("mana_focusing", (int8_t)0);
        w.node_mastery    = r.value("node_mastery", (int8_t)0);
        w.famous          = r.value("famous", (int8_t)0);
        w.runemaster      = r.value("runemaster", (int8_t)0);
        w.conjurer        = r.value("conjurer", (int8_t)0);
        w.charismatic     = r.value("charismatic", (int8_t)0);
        w.artificer       = r.value("artificer", (int8_t)0);
    }

    if (j.contains("spells_b64")) {
        auto decoded = base64_decode(j["spells_b64"].get<std::string>());
        std::memcpy(w.spells_list, decoded.data(),
                    std::min(decoded.size(), (size_t)NUM_SPELLS));
    }

    return w;
}

// Build the JSON object for a scenario (shared by file-based and buffer-based save).
// Powered by Claude.
static json scenario_to_json(const Scenario& sc) {
    json root;
    root["format_version"] = Scenario::FORMAT_VERSION;

    // Type
    switch (sc.type) {
        case ScenarioType::SANDBOX: root["scenario_type"] = "sandbox"; break;
        case ScenarioType::MISSION: root["scenario_type"] = "mission"; break;
        case ScenarioType::PUZZLE:  root["scenario_type"] = "puzzle";  break;
    }

    // Meta
    json meta;
    meta["title"]       = sc.meta.title;
    meta["author"]      = sc.meta.author;
    meta["description"] = sc.meta.description;
    meta["version"]     = sc.meta.version;
    meta["tags"]        = sc.meta.tags;
    root["meta"] = meta;

    // Game settings
    json gs;
    gs["difficulty"]   = sc.game_data.Difficulty;
    gs["magic"]        = sc.game_data.Magic;
    gs["land_size"]    = sc.game_data.Land_Size;
    gs["num_wizards"]  = sc.game_data.Total_Wizards;
    root["game_settings"] = gs;

    // Map generation parameters
    json mgp;
    mgp["land_proportion"] = sc.map_gen_params.land_proportion;
    mgp["roughness"]       = roughness_to_string(sc.map_gen_params.roughness);
    mgp["continent_count"] = sc.map_gen_params.continent_count;
    mgp["seed"]            = sc.map_gen_params.seed;
    mgp["mirror_planes"]   = sc.map_gen_params.mirror_planes;
    root["map_gen_params"]  = mgp;

    // World terrain (base64 encoded)
    json world;
    world["width"]  = WORLD_WIDTH;
    world["height"] = WORLD_HEIGHT;
    json planes = json::array();
    const char* plane_names[] = {"Arcanus", "Myrror"};
    for (int p = 0; p < NUM_PLANES; ++p) {
        json plane;
        plane["name"] = plane_names[p];
        plane["terrain"]   = base64_encode(
            reinterpret_cast<const uint8_t*>(sc.world.terrain[p]),
            WORLD_SIZE * sizeof(uint16_t));
        plane["specials"]  = base64_encode(
            reinterpret_cast<const uint8_t*>(sc.world.specials[p]),
            WORLD_SIZE);
        plane["flags"]     = base64_encode(
            sc.world.flags[p], WORLD_SIZE);
        plane["landmasses"] = base64_encode(
            sc.world.landmasses[p], WORLD_SIZE);
        planes.push_back(plane);
    }
    world["planes"] = planes;
    root["world"] = world;

    // Wizards
    json wiz_arr = json::array();
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (sc.wizards[i].is_active())
            wiz_arr.push_back(wizard_to_json(sc.wizards[i]));
    }
    root["wizards"] = wiz_arr;

    // Cities (only active)
    json city_arr = json::array();
    for (auto& c : sc.cities) {
        if (c.is_active()) city_arr.push_back(city_to_json(c));
    }
    root["cities"] = city_arr;

    // Units (only active)
    json unit_arr = json::array();
    for (auto& u : sc.units) {
        if (u.is_active()) unit_arr.push_back(unit_to_json(u));
    }
    root["units"] = unit_arr;

    // Nodes
    json node_arr = json::array();
    for (auto& n : sc.nodes) {
        json nj;
        nj["x"] = n.wx; nj["y"] = n.wy; nj["plane"] = n.wp;
        nj["owner"] = n.owner_idx; nj["power"] = n.power; nj["type"] = n.type;
        node_arr.push_back(nj);
    }
    root["nodes"] = node_arr;

    // Lairs (only populated)
    json lair_arr = json::array();
    for (auto& l : sc.lairs) {
        if (l.type == 0 && l.wx == 0 && l.wy == 0) continue;
        json lj;
        lj["x"] = l.wx; lj["y"] = l.wy; lj["plane"] = l.wp;
        lj["type"] = l.type; lj["intact"] = l.Intact;
        lj["guard1_type"] = l.guard1_unit_type; lj["guard1_count"] = l.guard1_count;
        lj["guard2_type"] = l.guard2_unit_type; lj["guard2_count"] = l.guard2_count;
        lj["loot_gold"] = l.Loot_Gold; lj["loot_mana"] = l.Loot_Mana;
        lair_arr.push_back(lj);
    }
    root["lairs"] = lair_arr;

    return root;
}

// Parse a JSON object into a scenario (shared by file-based and buffer-based load).
// Powered by Claude.
static bool json_to_scenario(const json& root, Scenario& sc) {
    sc.clear();

    // Type
    std::string type_str = root.value("scenario_type", "sandbox");
    if (type_str == "mission")     sc.type = ScenarioType::MISSION;
    else if (type_str == "puzzle") sc.type = ScenarioType::PUZZLE;
    else                           sc.type = ScenarioType::SANDBOX;

    // Meta
    if (root.contains("meta")) {
        auto& m = root["meta"];
        sc.meta.title       = m.value("title", "Untitled");
        sc.meta.author      = m.value("author", "");
        sc.meta.description = m.value("description", "");
        sc.meta.version     = m.value("version", "1.0.0");
        if (m.contains("tags"))
            sc.meta.tags = m["tags"].get<std::vector<std::string>>();
    }

    // Game settings
    if (root.contains("game_settings")) {
        auto& gs = root["game_settings"];
        sc.game_data.Difficulty    = gs.value("difficulty", (uint16_t)DIFF_NORMAL);
        sc.game_data.Magic         = gs.value("magic", (uint16_t)1);
        sc.game_data.Land_Size     = gs.value("land_size", (uint16_t)1);
        sc.game_data.Total_Wizards = gs.value("num_wizards", (uint16_t)2);
    }

    // Map generation parameters
    if (root.contains("map_gen_params")) {
        auto& mgp = root["map_gen_params"];
        sc.map_gen_params.land_proportion = mgp.value("land_proportion", 0.4f);
        sc.map_gen_params.roughness       = string_to_roughness(mgp.value("roughness", std::string("MEDIUM")));
        sc.map_gen_params.continent_count = mgp.value("continent_count", 3);
        sc.map_gen_params.seed            = mgp.value("seed", (uint32_t)0);
        sc.map_gen_params.mirror_planes   = mgp.value("mirror_planes", false);
    }

    // World
    if (root.contains("world") && root["world"].contains("planes")) {
        auto& planes = root["world"]["planes"];
        for (int p = 0; p < NUM_PLANES && p < (int)planes.size(); ++p) {
            auto& plane = planes[p];
            if (plane.contains("terrain")) {
                auto decoded = base64_decode(plane["terrain"].get<std::string>());
                std::memcpy(sc.world.terrain[p], decoded.data(),
                    std::min(decoded.size(), (size_t)(WORLD_SIZE * sizeof(uint16_t))));
            }
            if (plane.contains("specials")) {
                auto decoded = base64_decode(plane["specials"].get<std::string>());
                std::memcpy(sc.world.specials[p], decoded.data(),
                    std::min(decoded.size(), (size_t)WORLD_SIZE));
            }
            if (plane.contains("flags")) {
                auto decoded = base64_decode(plane["flags"].get<std::string>());
                std::memcpy(sc.world.flags[p], decoded.data(),
                    std::min(decoded.size(), (size_t)WORLD_SIZE));
            }
            if (plane.contains("landmasses")) {
                auto decoded = base64_decode(plane["landmasses"].get<std::string>());
                std::memcpy(sc.world.landmasses[p], decoded.data(),
                    std::min(decoded.size(), (size_t)WORLD_SIZE));
            }
        }
    }

    // Wizards
    if (root.contains("wizards")) {
        int idx = 0;
        for (auto& wj : root["wizards"]) {
            if (idx >= NUM_PLAYERS) break;
            sc.wizards[idx] = wizard_from_json(wj);
            ++idx;
        }
    }

    // Cities
    if (root.contains("cities")) {
        int idx = 0;
        for (auto& cj : root["cities"]) {
            if (idx >= NUM_CITIES) break;
            sc.cities[idx] = city_from_json(cj);
            ++idx;
        }
    }

    // Units
    if (root.contains("units")) {
        int idx = 0;
        for (auto& uj : root["units"]) {
            if (idx >= NUM_UNITS) break;
            sc.units[idx] = unit_from_json(uj);
            ++idx;
        }
    }

    // Nodes
    if (root.contains("nodes")) {
        int idx = 0;
        for (auto& nj : root["nodes"]) {
            if (idx >= NUM_NODES) break;
            auto& n = sc.nodes[idx];
            n.wx = nj.value("x", (int8_t)0);
            n.wy = nj.value("y", (int8_t)0);
            n.wp = nj.value("plane", (int8_t)0);
            n.owner_idx = nj.value("owner", (int8_t)-1);
            n.power = nj.value("power", (int8_t)0);
            n.type  = nj.value("type", (int8_t)0);
            ++idx;
        }
    }

    // Lairs
    if (root.contains("lairs")) {
        int idx = 0;
        for (auto& lj : root["lairs"]) {
            if (idx >= NUM_LAIRS) break;
            auto& l = sc.lairs[idx];
            l.wx = lj.value("x", (int8_t)0);
            l.wy = lj.value("y", (int8_t)0);
            l.wp = lj.value("plane", (int8_t)0);
            l.type = lj.value("type", (int8_t)0);
            l.Intact = lj.value("intact", (int8_t)1);
            l.guard1_unit_type = lj.value("guard1_type", (uint8_t)0);
            l.guard1_count     = lj.value("guard1_count", (uint8_t)0);
            l.guard2_unit_type = lj.value("guard2_type", (uint8_t)0);
            l.guard2_count     = lj.value("guard2_count", (uint8_t)0);
            l.Loot_Gold = lj.value("loot_gold", (int16_t)0);
            l.Loot_Mana = lj.value("loot_mana", (int16_t)0);
            ++idx;
        }
    }

    // Update totals
    sc.game_data.Total_Cities = static_cast<uint16_t>(sc.count_active_cities());
    sc.game_data.Total_Units  = static_cast<uint16_t>(sc.count_active_units());

    return true;
}

bool save_scenario(const std::string& path, const Scenario& sc) {
    json root = scenario_to_json(sc);

    // Write
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << root.dump(2);
    return f.good();
}

bool load_scenario(const std::string& path, Scenario& sc) {
    std::ifstream f(path);
    if (!f.is_open()) return false;

    json root;
    try {
        f >> root;
    } catch (...) {
        return false;
    }

    return json_to_scenario(root, sc);
}

// Serialize a scenario to a JSON byte buffer for use with platform file dialogs.
// Powered by Claude.
std::vector<uint8_t> serialize_scenario(const Scenario& sc) {
    json root = scenario_to_json(sc);
    std::string s = root.dump(2);
    return std::vector<uint8_t>(s.begin(), s.end());
}

// Deserialize a scenario from a JSON byte buffer loaded via platform file dialogs.
// Powered by Claude.
bool deserialize_scenario(const std::vector<uint8_t>& data, Scenario& sc) {
    if (data.empty()) return false;

    json root;
    try {
        std::string s(data.begin(), data.end());
        root = json::parse(s);
    } catch (...) {
        return false;
    }

    return json_to_scenario(root, sc);
}

} // namespace mom
