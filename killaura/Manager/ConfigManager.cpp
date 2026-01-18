#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <iomanip>
#include "../imgui_gl/imgui.h"
#include "../cheat/cheat.h"
#include "../cheat/EntityCache.h"
#include "../cheat/Esp.h"
#include "../nlohmann/json.hpp"

using json = nlohmann::json;

//json类型转换扩展
namespace nlohmann {
    // ImVec2 序列化: [x, y]
    template <>
    struct adl_serializer<ImVec2> {
        static void to_json(json& j, const ImVec2& v) {
            j = json{ v.x, v.y };
        }
        static void from_json(const json& j, ImVec2& v) {
            if (j.is_array() && j.size() >= 2) {
                v.x = j[0].get<float>();
                v.y = j[1].get<float>();
            }
        }
    };

    // ImColor 序列化: [r, g, b, a]
    template <>
    struct adl_serializer<ImColor> {
        static void to_json(json& j, const ImColor& c) {
            // ImColor 内部存储为 Value (ImVec4)
            j = json{ c.Value.x, c.Value.y, c.Value.z, c.Value.w };
        }
        static void from_json(const json& j, ImColor& c) {
            if (j.is_array() && j.size() >= 4) {
                c = ImColor(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>());
            }
        }
    };
}

namespace color {
    void RanbowChange() {
        static ImVec4& col = color::彩虹.Value;

        //变换速度
        const float speed = 0.0020f * Cg_Color::RainbowChangeSpeed;

        //获取时间戳
        float time = (*ct_var::lastmillis) * speed;

        //利用正弦波计算 RGB
        //sin 的结果是 -1 到 1，我们需要把它映射到 0 到 1
        //公式：(sin(t) * 0.5) + 0.5
        //相位偏移：Red=0, Green=2.094 (2π/3), Blue=4.188 (4π/3)
        float r = (std::sin(time) * 0.5f) + 0.5f;
        float g = (std::sin(time + 2.094f) * 0.5f) + 0.5f;
        float b = (std::sin(time + 4.188f) * 0.5f) + 0.5f;

        col.x = r;
        col.y = g;
        col.z = b;
    }
}

//辅助：获取带参数值的格式化字符串
std::string fmt_val(float val) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << val;
    return ss.str();
}

//存放函数
namespace Config {
    //初始化，读取配置文件
    void Init() {
		LoadConfig();
        //UpdateFuncString();//在Init里初始化字符串，长度排序会失效，因为此时ImGui还没初始化好

        FuncChange = true;//让Update函数去初始化字符串
    }
	//更新
	void update() {
		//更新窗口大小
		Config::window_size = ImGui::GetIO().DisplaySize;
        if (Config::FuncChange) {
            UpdateFuncString();//更新功能字符串
            if (Config::AutoSave) Config::SaveConfig();//处理自动保存
            Config::FuncChange = false;
        }
	}
	//读取配置文件
    void LoadConfig() {
        std::ifstream i(config_path);
        if (!i.is_open()) {
            //没有配置文件就保存一份默认的
            SaveConfig();
            return;
        }

        try {
            json j;
            i >> j;  

			//功能开关 配置加载
            if (j.contains("Function")) {
				auto& f = j["Function"];
				if (f.contains("H_BulletTrace")) Function::H_BulletTrace = f["H_BulletTrace"];
                if (f.contains("H_ShootDelay")) Function::H_ShootDelay = f["H_ShootDelay"];
                if (f.contains("H_BotFire")) Function::H_BotFire = f["H_BotFire"];
                if (f.contains("H_NoRecoil")) Function::H_NoRecoil = f["H_NoRecoil"];
                if (f.contains("H_Fly")) Function::H_Fly = f["H_Fly"];
                if (f.contains("H_Collide")) Function::H_Collide = f["H_Collide"];
                if (f.contains("H_Gravity")) Function::H_Gravity = f["H_Gravity"];
                if (f.contains("H_Speed")) Function::H_Speed = f["H_Speed"];
                if (f.contains("H_AutoJump")) Function::H_AutoJump = f["H_AutoJump"];
                if (f.contains("H_AbsoluteControl")) Function::H_AbsoluteControl = f["H_AbsoluteControl"];
                if (f.contains("H_MoveRecovery")) Function::H_MoveRecovery = f["H_MoveRecovery"];
                if (f.contains("H_BotMove")) Function::H_BotMove = f["H_BotMove"];
                if (f.contains("H_Rotate")) Function::H_Rotate = f["H_Rotate"];

                if (f.contains("E_AutoFire")) Function::E_AutoFire = f["E_AutoFire"];
            }

            //娱乐功能配置加载
            if (j.contains("Recreation")) {
                auto& r = j["Recreation"];
                if (r.contains("setDistance")) Recreation::setDistance = r["setDistance"];
                if (r.contains("setDeviation1")) Recreation::setDeviation1 = r["setDeviation1"];
                if (r.contains("setDeviation2")) Recreation::setDeviation2 = r["setDeviation2"];
            }
            
            //Move 配置加载
            if (j.contains("Move")) {
                auto& m = j["Move"];
                if (m.contains("ThrowMode")) Move::ThrowMode = static_cast<ThroughWallMode>(m["ThrowMode"].get<int>());
                if (m.contains("Speed")) Move::Speed = m["Speed"];
                if (m.contains("High")) Move::High = m["High"];
            }

			//AimBot 配置加载
            if (j.contains("AimBot")) {
                auto& a = j["AimBot"];
                if (a.contains("Enable")) AimBot::Enable = a["Enable"];
                if (a.contains("key")) AimBot::key = a["key"];
                if (a.contains("mode")) AimBot::mode = static_cast<AimMode>(a["mode"].get<int>());
                if (a.contains("smooth")) AimBot::smooth = a["smooth"];
			}

			//Esp 配置加载
            if (j.contains("Esp")) {
                auto& e = j["Esp"];
                if (e.contains("show_box")) Esp::show_box = e["show_box"];
                if (e.contains("show_name")) Esp::show_name = e["show_name"];
                if (e.contains("show_blood")) Esp::show_blood = e["show_blood"];
                if (e.contains("show_distance")) Esp::show_distance = e["show_distance"];
                if (e.contains("show_team")) Esp::show_team = e["show_team"];
                if (e.contains("show_line")) Esp::show_line = e["show_line"];
                if (e.contains("show_TargetInfo")) Esp::show_TargetInfo = e["show_TargetInfo"];
                if (e.contains("show_BulletImpact")) Esp::show_BulletImpact = e["show_BulletImpact"];
                if (e.contains("show_PlayerNum")) Esp::show_PlayerNum = e["show_PlayerNum"]; 
                if (e.contains("show_FunsState")) Esp::show_FunsState = e["show_FunsState"];
                if (e.contains("AMode")) Esp::AMode = static_cast<AlignMode>(e["AMode"].get<int>());
                if (e.contains("FuncPos")) Esp::FuncPos = e["FuncPos"].get<ImVec2>();
                
                if (e.contains("VisualJudgment")) Esp::VisualJudgment = e["VisualJudgment"]; 
                    

				//方框类型加载
                if (e.contains("BoxType")) Esp::boxType = static_cast<EspBoxType>(e["BoxType"].get<int>());

                if (e.contains("InfoPos")) Esp::InfoPos = e["InfoPos"].get<ImVec2>();
                if (e.contains("infoText")) Esp::infoText = e["infoText"];

            }

            //通用 配置加载
            if (j.contains("general")) {
                auto& g = j["general"];
                if (g.contains("filter_circle")) general::filter_circle = g["filter_circle"];
                if (g.contains("filter_teammate")) general::filter_teammate = g["filter_teammate"];
                if (g.contains("filter_ghost")) general::filter_ghost = g["filter_ghost"];
                if (g.contains("filter_dead")) general::filter_dead = g["filter_dead"];
                if (g.contains("filter_invisible")) general::filter_invisible = g["filter_invisible"];
                if (g.contains("filter_inscreen")) general::filter_inscreen = g["filter_inscreen"];
            }

			//combat 配置加载
            if (j.contains("combat")) {
                auto& c = j["combat"];
                if (c.contains("circle_radius")) combat::circle_radius = c["circle_radius"];
                if (c.contains("CTMode")) combat::CTMode = static_cast<choiceMode>(c["CTMode"].get<int>()); 
                if (c.contains("shoot_delay")) combat::shoot_delay = c["shoot_delay"];
            }
            
            // 颜色 配置加载
            if (j.contains("Color")) {
                auto& c = j["Color"];
                if (c.contains("Esp_color")) Color::Esp_color = c["Esp_color"].get<ImColor>();
                if (c.contains("Esp_color_mode")) Color::Esp_color_mode = static_cast<ColorMode>(c["Esp_color_mode"].get<int>());

                if (c.contains("Esp_targetColor")) Color::Esp_targetColor = c["Esp_targetColor"].get<ImColor>();
                if (c.contains("Esp_targetColor_mode")) Color::Esp_targetColor_mode = static_cast<ColorMode>(c["Esp_targetColor_mode"].get<int>());

                if (c.contains("Esp_VisibleColor")) Color::Esp_VisibleColor = c["Esp_VisibleColor"].get<ImColor>();
                if (c.contains("Esp_VisibleColor_mode")) Color::Esp_VisibleColor_mode = static_cast<ColorMode>(c["Esp_VisibleColor_mode"].get<int>());

                if (c.contains("Esp_InVisibleColor")) Color::Esp_InVisibleColor = c["Esp_InVisibleColor"].get<ImColor>();
                if (c.contains("Esp_InVisibleColor_mode")) Color::Esp_InVisibleColor_mode = static_cast<ColorMode>(c["Esp_InVisibleColor_mode"].get<int>());

                if (c.contains("Esp_FuncTextColor")) Color::Esp_FuncTextColor = c["Esp_FuncTextColor"].get<ImColor>();
                if (c.contains("Esp_FuncTextColor_mode")) Color::Esp_FuncTextColor_mode = static_cast<ColorMode>(c["Esp_FuncTextColor_mode"].get<int>());

                if (c.contains("Esp_infoBackColor")) Color::Esp_infoBackColor = c["Esp_infoBackColor"].get<ImColor>();

                if (c.contains("Esp_infoTextColor")) Color::Esp_infoTextColor = c["Esp_infoTextColor"].get<ImColor>();

                if (c.contains("Combat_CircleColor")) Color::Combat_CircleColor = c["Combat_CircleColor"].get<ImColor>();
                if (c.contains("Combat_CircleColor_mode")) Color::Combat_CircleColor_mode = static_cast<ColorMode>(c["Combat_CircleColor_mode"].get<int>());
                
                if (c.contains("RainbowChangeSpeed")) Color::RainbowChangeSpeed = c["RainbowChangeSpeed"];
            }
            if (j.contains("AutoSave")) Config::AutoSave = j["AutoSave"];
        }
        catch (const std::exception& e) {
            MessageBoxA(0, e.what(), "读取配置错误", MB_OK);
            //printf("读取配置错误: %s\n", e.what());
        }
    }
	//保存配置文件
    void SaveConfig() {
        json j;
		//Function命名空间
        //复杂功能
		j["Function"]["H_BulletTrace"] = Function::H_BulletTrace;
        j["Function"]["H_ShootDelay"] = Function::H_ShootDelay;
        j["Function"]["H_BotFire"] = Function::H_BotFire;
        j["Function"]["H_NoRecoil"] = Function::H_NoRecoil;
        j["Function"]["H_Fly"] = Function::H_Fly;
		j["Function"]["H_Collide"] = Function::H_Collide;
        j["Function"]["H_Gravity"] = Function::H_Gravity; 
        j["Function"]["H_Speed"] = Function::H_Speed;
        j["Function"]["H_AutoJump"] = Function::H_AutoJump;
        j["Function"]["H_AbsoluteControl"] = Function::H_AbsoluteControl;
        j["Function"]["H_MoveRecovery"] = Function::H_MoveRecovery;
        j["Function"]["H_BotMove"] = Function::H_BotMove; 
        j["Function"]["H_Rotate"] = Function::H_Rotate;

        //简单功能
        j["Function"]["E_AutoFire"] = Function::E_AutoFire;
        
        //Recreation命名空间
        j["Recreation"]["setDistance"] = Recreation::setDistance;
        j["Recreation"]["setDeviation1"] = Recreation::setDeviation1;
        j["Recreation"]["setDeviation2"] = Recreation::setDeviation2;

        //Move命名空间
        j["Move"]["ThrowMode"] = static_cast<int>(Move::ThrowMode);
        j["Move"]["Speed"] = Move::Speed;
        j["Move"]["High"] = Move::High;

        //AimBot命名空间
        j["AimBot"]["Enable"] = AimBot::Enable;
        j["AimBot"]["key"] = AimBot::key;
        j["AimBot"]["mode"] = static_cast<int>(AimBot::mode);
        j["AimBot"]["smooth"] = AimBot::smooth;

        //Esp命名空间
        j["Esp"]["show_box"] = Esp::show_box;
        j["Esp"]["show_name"] = Esp::show_name;
        j["Esp"]["show_blood"] = Esp::show_blood;
        j["Esp"]["show_distance"] = Esp::show_distance;
        j["Esp"]["show_team"] = Esp::show_team;
        j["Esp"]["show_line"] = Esp::show_line;
        j["Esp"]["show_TargetInfo"] = Esp::show_TargetInfo;
        j["Esp"]["BoxType"] = static_cast<int>(Esp::boxType); 
        j["Esp"]["show_BulletImpact"] = Esp::show_BulletImpact; 
        j["Esp"]["show_PlayerNum"] = Esp::show_PlayerNum;
        j["Esp"]["show_FunsState"] = Esp::show_FunsState;
        j["Esp"]["FuncPos"] = Esp::FuncPos;
        j["Esp"]["VisualJudgment"] = Esp::VisualJudgment; 

        j["Esp"]["infoText"] = Esp::infoText;
        j["Esp"]["InfoPos"] = Esp::InfoPos;
        j["Esp"]["AMode"] = Esp::AMode;

        //general命名空间
        j["general"]["filter_circle"] = general::filter_circle;
        j["general"]["filter_teammate"] = general::filter_teammate;
        j["general"]["filter_ghost"] = general::filter_ghost;
        j["general"]["filter_dead"] = general::filter_dead;
        j["general"]["filter_invisible"] = general::filter_invisible;
        j["general"]["filter_inscreen"] = general::filter_inscreen;

        //combat命名空间
        j["combat"]["circle_radius"] = combat::circle_radius;
        j["combat"]["shoot_delay"] = combat::shoot_delay;
        j["combat"]["CTMode"] = static_cast<int>(combat::CTMode);// Enum 需要强转为 int

        //color命名空间
        j["Color"]["Esp_color"] = Color::Esp_color;
        j["Color"]["Esp_color_mode"] = static_cast<int>(Color::Esp_color_mode);

        j["Color"]["Esp_targetColor"] = Color::Esp_targetColor;
        j["Color"]["Esp_targetColor_mode"] = static_cast<int>(Color::Esp_targetColor_mode);

        j["Color"]["Esp_VisibleColor"] = Color::Esp_VisibleColor;
        j["Color"]["Esp_VisibleColor_mode"] = static_cast<int>(Color::Esp_VisibleColor_mode);

        j["Color"]["Esp_InVisibleColor"] = Color::Esp_InVisibleColor;
        j["Color"]["Esp_InVisibleColor_mode"] = static_cast<int>(Color::Esp_InVisibleColor_mode);

        j["Color"]["Esp_FuncTextColor"] = Color::Esp_FuncTextColor;
        j["Color"]["Esp_FuncTextColor_mode"] = static_cast<int>(Color::Esp_FuncTextColor_mode);

        j["Color"]["Esp_infoBackColor"] = Color::Esp_infoBackColor;

        j["Color"]["Esp_infoTextColor"] = Color::Esp_infoTextColor;

        j["Color"]["Combat_CircleColor"] = Color::Combat_CircleColor;
        j["Color"]["Combat_CircleColor_mode"] = static_cast<int>(Color::Combat_CircleColor_mode);

        j["Color"]["RainbowChangeSpeed"] = Color::RainbowChangeSpeed;
        
        //系统设置，自动保存
        j["AutoSave"] = AutoSave;

        // --- 写入文件 ---
        std::ofstream o(config_path);
        if (o.is_open()) {
            // setw(4) 用于美化缩进
            o << std::setw(4) << j << std::endl;
            o.close();
        }
    }
    //功能名称字符串更新
    void UpdateFuncString() {
        //安全检查
        if (ImGui::GetCurrentContext() == nullptr) return;

        //临时存放字符串
        std::vector<std::string> activeList;

        // Function 命名空间
        if (Cg_Func::H_BulletTrace) activeList.push_back(u8"子弹追踪");
        if (Cg_Func::H_ShootDelay)  activeList.push_back(u8"射击延迟 [" + std::to_string(Cg_combat::shoot_delay) + "ms]");
        if (Cg_Func::H_BotFire)     activeList.push_back(u8"禁止机器人开火");
        if (Cg_Func::H_BotMove)     activeList.push_back(u8"禁止机器人移动");
        if (Cg_Func::H_NoRecoil)    activeList.push_back(u8"无后坐力");
        if (Cg_Func::H_Fly)         activeList.push_back(u8"飞行模式");

        if (Cg_Func::H_Collide) {
            std::string modeStr = u8"水平";
            if (Cg_Move::IsZ_through()) modeStr = u8"纵向";
            else if (Cg_Move::IsALL_through()) modeStr = u8"ALL";
            activeList.push_back(u8"穿墙模式 [" + modeStr + "]");
        }

        if (Cg_Func::H_Gravity)     activeList.push_back(u8"重力修改 [" + fmt_val(Cg_Move::Gravity) + "]");
        if (Cg_Func::H_Speed)       activeList.push_back(u8"加速 [" + fmt_val(Cg_Move::Speed) + "]");
        if (Cg_Func::H_AutoJump)    activeList.push_back(u8"自动跳跃 [高" + fmt_val(Cg_Move::High) + "]");
        if (Cg_Func::H_AbsoluteControl) activeList.push_back(u8"灵活控制");
        if (Cg_Func::H_MoveRecovery)    activeList.push_back(u8"无移动后摇");
        if (Cg_Func::H_Rotate)      activeList.push_back(u8"围绕目标旋转 [范围" + fmt_val(Cg_Recreation::setDistance) + "]");
        if (Cg_Func::E_AutoFire)    activeList.push_back(u8"自动开火");

        // AimBot 命名空间
        if (Cg_AimBot::Enable) {
            std::string mode = (Cg_AimBot::mode == AimMode::lock) ? u8"强锁" :
                u8"平滑:" + std::to_string(Cg_AimBot::smooth);
            activeList.push_back(u8"自动瞄准 [" + mode + "]");
        }

        // ESP 命名空间
        if (Cg_Esp::show_box || Cg_Esp::show_name || Cg_Esp::show_blood ||
            Cg_Esp::show_distance || Cg_Esp::show_team || Cg_Esp::show_line) {
            std::string espInfo = u8"透视";
            int cnt = 3;//最多显示3条
            espInfo += u8"[";
            if (cnt && Cg_Esp::show_box) {
                espInfo += u8"方框 ";
                --cnt;
            }
            if (cnt && Cg_Esp::show_name) {
                espInfo += u8"名字 ";
                --cnt;
            }
            if (cnt && Cg_Esp::show_blood) {
                espInfo += u8"血量 ";
                --cnt;
            }
            if (cnt && Cg_Esp::show_distance) {
                espInfo += u8"距离 ";
                --cnt;
            }
            if (cnt && Cg_Esp::show_team) {
                espInfo += u8"队伍 ";
                --cnt;
            }
            if (cnt && Cg_Esp::show_line) {
                espInfo += u8"射线 ";
                --cnt;
            }

            espInfo += u8"]";
            activeList.push_back(espInfo);
        }
        if (Cg_Esp::VisualJudgment) {
            activeList.push_back(u8"可视化标注");
        }

        //排序
        //如果字体未加载就按字符数量来排序
        //加载了就按实际宽度比较大小
        std::sort(activeList.begin(), activeList.end(), [](const std::string& a, const std::string& b) {
            if (ImGui::GetFont() == nullptr) return a.length() > b.length();
            return ImGui::CalcTextSize(a.c_str()).x > ImGui::CalcTextSize(b.c_str()).x;
            });

        //交换容器
        Cg_Esp::ActiveFuncList.swap(activeList);
    }
}
