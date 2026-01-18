#pragma once
#include <string>
#include <Windows.h>
#include <vector>
#include "../imgui_gl/imgui.h"
#include "../vec3/Vector.h"

#define Cg_AimBot Config::AimBot
#define Cg_Esp Config::Esp
#define Cg_general Config::general
#define Cg_combat Config::combat
#define Cg_Func Config::Function
#define Cg_Move Config::Move
#define Cg_Color Config::Color
#define Cg_Recreation Config::Recreation

//索敌模式
enum choiceMode {
    distance,//距离最近
    sight//准心最近
};
//自瞄模式
enum AimMode {
    lock,//强锁模式
	smooth//平滑模式
};
//透视方框类型
enum EspBoxType {
	rect2d,//2D方框
	edge2d,//2D边框
	threeD//3D方框
};
//穿墙模式
enum ThroughWallMode {
    XY_through,//水平穿墙
    Z_through,//纵向穿墙
    ALL_through//全向穿墙
};
//颜色模式
enum ColorMode {
    Customize,//自定义
    Rainbow//彩虹模式
};
//功能字符串对齐
enum AlignMode {
    LEFT,//文字向左对齐
    RIGHT//文字向右对齐
};

//颜色
namespace color {
    //绿色
    inline ImVec4 纯绿色 = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    inline ImVec4 半透明绿 = ImVec4(0.0f, 1.0f, 0.0f, 0.5f);
    inline ImVec4 深绿色 = ImVec4(0.0f, 0.5f, 0.0f, 1.0f);
    inline ImVec4 浅绿色 = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);

    //红色
    inline ImVec4 鲜红色 = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    inline ImVec4 暗红色 = ImVec4(0.5f, 0.0f, 0.0f, 1.0f);
    inline ImVec4 浅红色 = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
    inline ImVec4 漂亮红色 = ImColor(IM_COL32(220, 40, 40, 180));
    inline ImVec4 半透明红 = ImVec4(1.0f, 0.0f, 0.0f, 0.5f);

    //蓝色
    inline ImVec4 纯蓝色 = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    inline ImVec4 浅蓝色 = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);
    inline ImVec4 深蓝色 = ImVec4(0.0f, 0.0f, 0.5f, 1.0f);

    //黄色
    inline ImVec4 纯黄色 = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    inline ImVec4 金色 = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
    inline ImVec4 琥珀色 = ImVec4(1.0f, 0.75f, 0.0f, 1.0f);

    //紫色
    inline ImVec4 紫色 = ImVec4(0.5f, 0.0f, 0.5f, 1.0f);
    inline ImVec4 紫罗兰色 = ImVec4(0.58f, 0.0f, 0.83f, 1.0f);

    //霓虹色系
    inline ImVec4 霓虹绿 = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
    inline ImVec4 霓虹蓝 = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);

    //基础色
    inline ImVec4 白色 = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    inline ImVec4 黑色 = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    inline ImVec4 灰色 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

    //特殊效果色
    inline ImVec4 半透明白 = ImVec4(1.0f, 1.0f, 1.0f, 0.3f);
    inline ImVec4 半透明黑 = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
    inline ImVec4 透明色 = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    //彩虹
    inline ImColor 彩虹 = ImVec4(0, 0, 0, 1.0f);

    //彩虹变换颜色
    void RanbowChange();
}

//存放各种配置开关和参数
namespace Config {
    //功能开关配置
    namespace Function {
		//复杂功能，通常需要Hook函数（寄生在游戏流程里）
        //通过游戏被动执行，个别各功能需要写入补丁
		inline bool H_BulletTrace = false;//子弹追踪开关
        inline bool H_ShootDelay = false;//射击延迟修改开关
        inline bool H_BotFire = false;//机器人禁止开火开关
        inline bool H_NoRecoil = false;//无后坐力开关
        inline bool H_Fly = false;//飞行开关
        inline bool H_Collide = false;//穿墙控制
        inline bool H_Gravity = false;//重力修改开关
        inline bool H_Speed = false;//速度设置开关
        inline bool H_AutoJump = false;//自动跳跃
        inline bool H_AbsoluteControl = false;//灵活控制人物开关
        inline bool H_MoveRecovery = false;//无后摇开关
        inline bool H_BotMove = false;//机器人禁止移动开关
        inline bool H_Rotate = false;//围绕目标旋转

        //普通功能，只需要按需执行函数，由我们自己的代码调用
        //主动执行
		inline bool E_AutoFire = false;//自动开火开关
    }
    //娱乐功能配置
    namespace Recreation {
        inline Vector3* point = nullptr;//跟随目标的点
        inline float setDistance = 5.0f;//跟随距离
        inline float setDeviation1 = 2.0f;//旋转偏差范围
        inline float setDeviation2 = 5.0f;//加速范围
    }
    //移动功能相关的配置
    namespace Move {
        inline float Gravity{};//重力值
        inline float Speed = 1.0f;//速度倍数
        inline float High = 20.0f;//跳跃高度
        inline ThroughWallMode ThrowMode = ThroughWallMode::XY_through;//穿墙模式

        //辅助判断模式的函数
        inline bool IsZ_through() { return ThrowMode == ThroughWallMode::Z_through; };
        inline bool IsXY_through() { return ThrowMode == ThroughWallMode::XY_through; };
        inline bool IsALL_through() { return ThrowMode == ThroughWallMode::ALL_through; };
    }
    //自瞄配置
    namespace AimBot {
		inline bool Enable = false;//自动瞄准开关
		inline short key = VK_RBUTTON;//自动瞄准按键(默认鼠标右键)
		inline AimMode mode = AimMode::smooth;//瞄准模式（默认平滑模式）
		inline int smooth = 5;//瞄准平滑度（数值越大越慢）
    }
    //透视功能配置
	namespace Esp {
		//透视功能开关
        inline bool show_box = false;
        inline bool show_name = false;
        inline bool show_blood = false;
        inline bool show_distance = false;
		inline bool show_team = false;
        inline bool show_line = false;//从游戏顶部与敌人头部连线
		inline bool show_TargetInfo = false;//显示最佳目标信息
        inline bool show_BulletImpact = false;//显示子弹落点
        inline bool show_PlayerNum = false;//显示局内人数情况，人数，可见状态
        inline bool show_FunsState = false;//显示功能开启情况
        inline bool VisualJudgment = false;//对不可见和可见的敌人用不同的颜色进行标注

		//透视方框类型
		inline EspBoxType boxType = EspBoxType::rect2d;

        //功能名称总字符串列表
        inline std::vector<std::string> ActiveFuncList;
        //功能名称对齐方式
        inline AlignMode AMode = AlignMode::RIGHT;
        //功能显示的左上角坐标！
        inline ImVec2 FuncPos = { 0,0 };

		//最佳敌人信息窗口相关配置
		inline std::string infoText = "";//信息内容
		inline ImVec2 InfoPos = { 0,0 };//实际信息窗口左上角位置！Config::update负责更新
	}
    //通用配置：透视和其他战斗选项都要读取的配置！
    namespace general {
        inline bool filter_circle = true;//过滤敌人的圈！
        inline bool filter_teammate = true;//过滤队友
        inline bool filter_ghost = true;//过滤幽灵
        inline bool filter_dead = true;//过滤死人
		inline bool filter_invisible = false;//过滤不可见敌人  目前仅在选取最佳目标时使用
        inline bool filter_inscreen = false;//过滤身后敌人，不存在于屏幕的敌人
    }
	//战斗相关配置
    namespace combat {
        inline float circle_radius = 50.0f;//过滤圈的半径

        //自定义射击延迟
        inline int shoot_delay{};

		//最佳目标选择模式
        inline choiceMode CTMode = choiceMode::distance;
    }
    //颜色配置
    namespace Color {
        //默认颜色
        inline ImColor Esp_color = color::白色;//透视功能的颜色
        inline ColorMode Esp_color_mode = ColorMode::Customize;

        inline ImColor Esp_targetColor = color::暗红色;//最佳目标颜色
        inline ColorMode Esp_targetColor_mode = ColorMode::Customize;

        inline ImColor Esp_VisibleColor = color::黑色;//可见的实体颜色
        inline ColorMode Esp_VisibleColor_mode = ColorMode::Customize;

        inline ImColor Esp_InVisibleColor = color::白色;//不可见的实体颜色
        inline ColorMode Esp_InVisibleColor_mode = ColorMode::Customize;

        inline ImColor Esp_FuncTextColor = color::紫色;//功能显示文字颜色
        inline ColorMode Esp_FuncTextColor_mode = ColorMode::Customize;

        inline ImColor Esp_infoBackColor = color::漂亮红色;//信息窗口背景颜色

        inline ImColor Esp_infoTextColor = color::白色;//信息窗口文字颜色

        inline ImColor Combat_CircleColor = color::暗红色;//过滤圈颜色
        inline ColorMode Combat_CircleColor_mode = ColorMode::Customize;

        inline float RainbowChangeSpeed = 1.0f;//彩虹变换速度
    }
}
//存放系统配置的变量
namespace Config {
    //游戏窗口大小
    inline ImVec2 window_size;
    //功能变化标志位
    inline bool FuncChange = false;
    //自动保存开关
    inline bool AutoSave = false;
    //配置文件路径
    inline std::string config_path = ".\\killaura_config.ini";
}
//存放函数
namespace Config{
    //初始化，读取配置文件
    void Init();
    //更新窗口大小和一些配置
    void update();
    //读取配置文件
    void LoadConfig();
    //保存配置文件
    void SaveConfig();
    //功能名称字符串更新函数
    void UpdateFuncString();
}