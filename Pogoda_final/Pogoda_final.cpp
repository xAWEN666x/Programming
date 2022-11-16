#include <iostream>
#include <string>
#include <fstream>
#include <string>
#include <math.h> 
#include "include/json.hpp"
#include "include/httplib.h"
using namespace httplib;
using json = nlohmann::json;
using namespace std;

json weather;

void re_wet() // Кэш погоды
{
    Client cliw("api.openweathermap.org");
    auto wet = cliw.Get("/data/2.5/forecast?q=Simferopol&units=metric&lang=ru&appid=bebffc0c3258a19f3f99e6f2d64579e3");
    weather = json::parse(wet->body);
    ofstream wet_cache("include/cache.json");
    wet_cache << weather;
    wet_cache.close();
}
int find_nice_time(int current_time) // Проверка актуального времени 
{
    int nice_time = -1;
    for (int i = 0; i < weather["list"].size(); i++) {

        if (current_time < weather["list"][i]["dt"])
        {
            nice_time = i;
            break;
        }
        nice_time = -1;
    }
    return nice_time;
}
void api_response(const Request& req, Response& res) { 
    Client time_cli("http://worldtimeapi.org");
    auto time = time_cli.Get("/api/timezone/europe/simferopol");
    json j = json::parse(time->body);
    int current_time = j["unixtime"];
    ifstream wet_caсhe("include/cache.json");
    string wet_str;
    getline(wet_caсhe, wet_str, '\0');
    weather = json::parse(wet_str);
    wet_caсhe.close();
    int nice_time = find_nice_time(current_time);
    if (nice_time == -1)
    {
        re_wet();
        nice_time = find_nice_time(current_time);
    }
    json j1;
    j1["temp"] = weather["list"][nice_time]["main"]["temp"].get<double>();
    j1["description"] = weather["list"][nice_time]["weather"][0]["description"];
    res.set_content(j1.dump(2), "application/json"); //Вывод актуальных данных
}
void gen_response(const Request& req, Response& res) {
    Client time_cli("http://worldtimeapi.org");
    auto time = time_cli.Get("/api/timezone/europe/simferopol");
    json j = json::parse(time->body);
    int current_time = j["unixtime"];
    if (time) {
        if (time->status == 200) {

        }
        else {
            std::cout << "Status code: " << time->status << std::endl;
        }
    }
    else {
        auto err = time.error();
        std::cout << "Error of time code: " << err << std::endl;
    }
    ifstream wet_caсhe("include/cache.json");
    string wet_str = "";
    getline(wet_caсhe, wet_str, '\0');
    weather = json::parse(wet_str);
    wet_caсhe.close();
    int nice_time = find_nice_time(current_time);
    if (nice_time == -1)
    {
        re_wet();
        nice_time = find_nice_time(current_time);
    }
    ifstream file("include/widget_template.html"); // Файловая переменная
    string str;                 // Буфер. Тут будет текст файла
    getline(file, str, '\0');
    int description;
    int temp;
    int icon;
    description = str.find("{list[i].weather[0].description}");
    str.replace(description, 32, weather["list"][nice_time]["weather"][0]["description"]);      
    icon = str.find("{list[i].weather[0].icon}");      
    str.replace(icon, 25, weather["list"][nice_time]["weather"][0]["icon"]);
    temp = str.find("{list[i].main.temp}"); 
    str.replace(temp, 19, to_string((int)round(weather["list"][nice_time]["main"]["temp"].get<double>())));
    temp = str.find("{list[i].main.temp}");
    str.replace(temp, 19, to_string((int)round(weather["list"][nice_time]["main"]["temp"].get<double>())));
    res.set_content(str, "text/html");// Вывод виджета
}
int main() {
    setlocale(LC_ALL, "Russian");
    Server svr;
    svr.Get("/", gen_response); //Виджет
    svr.Get("/api", api_response);  // Данные
    std::cout << "Start server... OK\n";
    svr.listen("localhost", 3000);
}