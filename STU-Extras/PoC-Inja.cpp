
// #pragma warning(suppress : 4703)
// 
// Save warning levels, and drop it to level 3 
#pragma warning (push, 3)

// turn specific warnings off
// #pragma warning (disable : 4701 4703 6001)
#pragma warning (disable : 4244)  // Warning   C4244    'initializing': conversion from '...' to '...', possible loss of data

// screwy code goes here
#include <inja/inja.hpp>

// restore original warning levels.
#pragma warning (pop)

// #include <inja/inja.hpp>
// NOTE  Inja uses the string_view feature of the C++17 STL.

#include "STU-Extras.hpp"


void poc_inja(void)
{

    inja::json data;
    data["name"] = "world";

    inja::render("Hello {{ name }}!", data); // Returns std::string "Hello world!"
    inja::render_to(std::cout, "Hello {{ name }}!", data); // Writes "Hello world!" to stream


  //  nlohmann::json json4;
  //  json4["name"] = "NULL";
  //  std::string template5 = "INSERT INTO [my_table] (col1) VALUES ( \
  //{%if name == \"NULL\"%}\
  //{{name}}\
  //{%else%}\
  //'{{name}}'\
  //{%endif%}\
  //)";
  //  std::string rendered5 = inja::render(template5, json4);
  //  std::cout << rendered5 << std::endl;


    inja::json unittype_json;

    // unittype_json = inja::json::object();

    unittype_json["unittype_00"]["name"] = "Barbarian";

    // unittype_json["unittype_00"] = "Barbarian";

    std::string unittype_template = "{ }, ";

    std::string unittype_rendered = inja::render(unittype_template, unittype_json);
    std::cout << unittype_rendered << std::endl;


    std::ofstream unittype_file;
    unittype_file.open("unittype.json");
    inja::render_to(unittype_file, "Hello {{ name }}!", data);
    unittype_file.close();

    std::cout << std::setw(4) << unittype_json << std::endl;


}