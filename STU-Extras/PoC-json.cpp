
#include <nlohmann/json.hpp>
// using json = nlohmann::json;
// ...
// std::ifstream f("example.json");
// json data = json::parse(f);

#include "STU-Extras.hpp"



void poc_json(void)
{

    nlohmann::json data;
    data["name"] = "world";

    nlohmann::json unittype_json;

    // unittype_json = inja::json::object();

    unittype_json["unittype_00"]["name"] = "Barbarian";

    // unittype_json["unittype_00"] = "Barbarian";

    std::cout << std::setw(4) << unittype_json << std::endl;

}