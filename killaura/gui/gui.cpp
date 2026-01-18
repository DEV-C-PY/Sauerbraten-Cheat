#define IMGUI_DEFINE_MATH_OPERATORS
#include "gui.h"
#include "../imgui_gl/imgui.h"
#include "../cheat/cheat.h"
#include "../cheat/EntityCache.h"
#include "../cheat/Esp.h"
#include "../Manager/ConfigManager.h"
#include "../cheat/func_prototype.h"
#include "../cheat/Patch.h"
#include "../imgui_gl/imgui_internal.h"


using namespace std;

//动点测试，让点按矩形方向进行移动
//在gui::Debug()测试跟随旋转目标时候使用
//1.点的坐标	2.前进次数	3.多久前进一次	4.步进
void RectMove(Vector3& point, int cycle, int time, int step);

//菜单页面
namespace MenuPage {
	//主页
	void MainPage() {
		ImGui::BeginGroupPanel(u8"Killaura-Sauerbraten", ImVec2(-1, -1));
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
			ImGui::Text(u8"%s你好", ct_Player::name);
			ImGui::Text(u8"Home - 隐藏\\显示菜单");
			if(gui::windowFlag::click_lock)
				ImGui::TextColored(color::浅红色,u8"Insert - 解锁聚焦");
			else 
				ImGui::TextColored(color::半透明绿, u8"Insert - 聚焦菜单");
			ImGui::BeginGroupPanel(u8"", ImVec2(-1, 0));
			{
				ImGui::Text(u8"作品花费4个星期创作");
				ImGui::Spacing();
				ImGui::Text(u8"我写的第二个辅助");
				ImGui::Spacing();
				UI::TextURL(u8"项目地址", "https://github.com/DEV-C-PY/Sauerbraten-Cheat/tree/main");
				ImGui::Spacing();
			}
			ImGui::EndGroupPanel();

			ImGui::BeginGroupPanel(u8"", ImVec2(-1, 0));
			{
				ImGui::Text(u8"Created by wwa");
				ImGui::Spacing();
				ImGui::Text(u8"交流群:1080622291");
				ImGui::Spacing();
				ImGui::Text(u8"最后更新时间:2026/1/18");
				ImGui::Spacing();
			}
			ImGui::EndGroupPanel();
			ImGui::Spacing();
			ImGui::PopStyleVar(); // 恢复 Padding
		}
		ImGui::EndGroupPanel();
	}
	//战斗功能页面
	void CombatPage() {
		// 左右分栏布局
		ImGui::Columns(1, nullptr, false); // false 表示不显示中间的分隔线

		//上边自瞄组
		ImGui::BeginGroupPanel(u8"自瞄配置", ImVec2(-1, 0));
		{
			UI::ToggleButton(u8"开启自瞄", &Cg_AimBot::Enable);
			ImGui::Spacing();
			UI::RadioButton(u8"平滑模式", (int*)&Cg_AimBot::mode, AimMode::smooth); ImGui::SameLine();
			UI::RadioButton(u8"暴力锁头", (int*)&Cg_AimBot::mode, AimMode::lock);
			ImGui::Spacing();
			UI::SliderInt(u8"平滑数值", &Cg_AimBot::smooth, 1, 30);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();

		//中间武器控制
		ImGui::BeginGroupPanel(u8"武器控制", ImVec2(-1, 0));
		{
			UI::ToggleButton(u8"自定义开火延迟", &Cg_Func::H_ShootDelay); ImGui::SameLine();
			UI::ToggleButton(u8"无后坐力", &Cg_Func::H_NoRecoil);
			ImGui::Spacing();
			UI::SliderInt(u8"射击延迟", &Cg_combat::shoot_delay, 0, 2000);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();

		//下半边放其余功能
		ImGui::BeginGroupPanel(u8"其他", ImVec2(-1, 0));
		{
			UI::ToggleButton(u8"子弹追踪", &Cg_Func::H_BulletTrace); ImGui::SameLine(150);
			UI::ToggleButton(u8"自动开火", &Cg_Func::E_AutoFire);
			ImGui::Spacing();

			UI::ToggleButton(u8"禁止机器人开火", &Cg_Func::H_BotFire); ImGui::SameLine();
			UI::ToggleButton(u8"禁止机器人移动", &Cg_Func::H_BotMove);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();

		ImGui::Columns(1); // 结束分栏
	}
	//移动功能页面
	void MovePage() {
		ImGui::BeginGroupPanel(u8"移动", ImVec2(-1, 0));
		{
			if (UI::Checkbox(u8"穿墙", &Cg_Func::H_Collide)) {
				if (Cg_Func::H_Collide) Patch::ThroughWall::Update();
				else Patch::ThroughWall::AllDone();
			}ImGui::SameLine();
			if (UI::RadioButton(u8"水平穿墙", (int*)&Cg_Move::ThrowMode, (int)ThroughWallMode::XY_through)) {
				if (Cg_Func::H_Collide) Patch::ThroughWall::Update();
			}ImGui::SameLine();
			if (UI::RadioButton(u8"纵向穿墙", (int*)&Cg_Move::ThrowMode, (int)ThroughWallMode::Z_through)) {
				if (Cg_Func::H_Collide) Patch::ThroughWall::Update();
			}ImGui::SameLine();
			if (UI::RadioButton(u8"全向穿墙", (int*)&Cg_Move::ThrowMode, (int)ThroughWallMode::ALL_through)) {
				if (Cg_Func::H_Collide) Patch::ThroughWall::Update();
			}
			ImGui::Spacing();

			UI::ToggleButton(u8"重力修改", &Cg_Func::H_Gravity); ImGui::SameLine();
			UI::ToggleButton(u8"无移动后摇", &Cg_Func::H_MoveRecovery);
			ImGui::Spacing();

			if (UI::ToggleButton(u8"飞行", &Cg_Func::H_Fly)) {
				if (Cg_Func::H_Fly) Patch::Fly::CheckWall->Apply();
				else Patch::Fly::CheckWall->Restore();
			} ImGui::SameLine();
			UI::ToggleButton(u8"围绕目标旋转", &Cg_Func::H_Rotate);
			ImGui::Spacing();

			UI::ToggleButton(u8"加速", &Cg_Func::H_Speed); ImGui::SameLine();
			UI::ToggleButton(u8"自动跳跃", &Cg_Func::H_AutoJump); ImGui::SameLine();
			UI::ToggleButton(u8"灵活控制", &Cg_Func::H_AbsoluteControl);
			ImGui::Spacing();

		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(u8"移动配置", ImVec2(-1, 0));
		{
			UI::SliderFloat(u8"移动速度", &Cg_Move::Speed, 1.0f, 10.0f);
			ImGui::Spacing();
			UI::SliderFloat(u8"跳跃高度", &Cg_Move::High, 10.0f, 100.0f);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(u8"旋转配置", ImVec2(-1, 0));
		{
			UI::SliderFloat(u8"跟随距离", &Cg_Recreation::setDistance, 1.0f, 10.0f);
			ImGui::Spacing();
			UI::SliderFloat(u8"旋转误差", &Cg_Recreation::setDeviation1, 1.0f, 5.0f);
			ImGui::Spacing();
			UI::SliderFloat(u8"外层吸附范围", &Cg_Recreation::setDeviation2, 0.0f, 10.0f);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(u8"重力配置", ImVec2(-1, 0));
		{
			UI::SliderFloat(u8"重力值", &Cg_Move::Gravity, -200, 200);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();

	}
	//透视与视觉功能页面
	void EspPage() {
		ImGui::Columns(1, nullptr, false);

		ImGui::BeginGroupPanel(u8"绘制选项", ImVec2(-1, 0));
		{
			UI::ToggleButton(u8"方框", &Cg_Esp::show_box); ImGui::SameLine();
			UI::ToggleButton(u8"连线", &Cg_Esp::show_line); ImGui::SameLine();
			UI::ToggleButton(u8"距离", &Cg_Esp::show_distance); ImGui::SameLine();
			ImGui::Spacing();

			UI::ToggleButton(u8"血量", &Cg_Esp::show_blood); ImGui::SameLine();
			UI::ToggleButton(u8"队伍", &Cg_Esp::show_team); ImGui::SameLine();
			UI::ToggleButton(u8"名字", &Cg_Esp::show_name);
			ImGui::Spacing();

			UI::ToggleButton(u8"目标信息", &Cg_Esp::show_TargetInfo); ImGui::SameLine();
			UI::ToggleButton(u8"子弹落点", &Cg_Esp::show_BulletImpact);
			ImGui::Spacing();

			UI::ToggleButton(u8"可视化标注", &Cg_Esp::VisualJudgment); ImGui::SameLine();
			UI::ToggleButton(u8"局内人数", &Cg_Esp::show_PlayerNum);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(u8"方框样式", ImVec2(-1, 0));
		{
			UI::RadioButton(u8"2D方框", (int*)&Cg_Esp::boxType, EspBoxType::rect2d); ImGui::SameLine();
			UI::RadioButton(u8"2D边框", (int*)&Cg_Esp::boxType, EspBoxType::edge2d); ImGui::SameLine();
			UI::RadioButton(u8"3D方框", (int*)&Cg_Esp::boxType, EspBoxType::threeD);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(u8"视觉", ImVec2(-1, 0));
		{
			UI::ToggleButton(u8"显示开启功能", &Cg_Esp::show_FunsState); ImGui::SameLine();
			UI::ToggleButton(u8"调色板", &gui::windowFlag::EditColor);
			ImGui::Spacing();
			UI::SliderFloat(u8"彩虹变换速度", &Cg_Color::RainbowChangeSpeed, 1.0f, 6.0f);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();

		ImGui::Columns(1);
	}
	//设置页面
	void SetPage() {
		ImGui::BeginGroupPanel(u8"索敌模式", ImVec2(-1, 0));
		{
			UI::RadioButton(u8"距离最近", (int*)&Cg_combat::CTMode, choiceMode::distance); ImGui::SameLine(150);
			UI::RadioButton(u8"准心最近", (int*)&Cg_combat::CTMode, choiceMode::sight);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(u8"过滤配置", ImVec2(-1, 0));
		{
			UI::ToggleButton(u8"过滤圈外实体", &Cg_general::filter_circle); ImGui::SameLine();
			UI::ToggleButton(u8"过滤身后实体", &Cg_general::filter_inscreen);
			ImGui::Spacing();

			UI::ToggleButton(u8"过滤不可见实体", &Cg_general::filter_invisible); ImGui::SameLine();
			UI::ToggleButton(u8"过滤队友", &Cg_general::filter_teammate);
			ImGui::Spacing();

			UI::ToggleButton(u8"过滤幽灵", &Cg_general::filter_ghost); ImGui::SameLine();
			UI::ToggleButton(u8"过滤死人", &Cg_general::filter_dead);
			ImGui::Spacing();

			UI::SliderFloat(u8"过滤圈半径", &Cg_combat::circle_radius, 50.0f, 1000.0f);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(u8"系统设置", ImVec2(-1, 0));
		{
			if (ImGui::Button(u8"保存配置文件", ImVec2(-1, 30))) {
				Config::SaveConfig();
			}
			ImGui::Spacing();
			UI::ToggleButton(u8"自动保存", &Config::AutoSave);
			ImGui::Spacing();
		}
		ImGui::EndGroupPanel();
	}
}
//绘制
namespace gui {
	//主题设置
	void SetupStyle() {
		ImGuiStyle& style = ImGui::GetStyle();

		// 造型定制
		style.Alpha = 1.0f;
		style.WindowRounding = 10.0f;    // 窗口大圆角
		style.ChildRounding = 8.0f;      // 子版块圆角
		style.FrameRounding = 6.0f;      // 控件(Checkbox等)圆角
		style.GrabRounding = 6.0f;       // 滑块圆角
		style.PopupRounding = 8.0f;
		style.ScrollbarRounding = 8.0f;
		style.ScrollbarSize = 14.0f;     // 滚动条变细一点

		style.ItemSpacing = ImVec2(10, 10);      // 控件之间的距离变大，呼吸感
		style.WindowPadding = ImVec2(20, 20);    // 窗口内边距

		// 颜色定制
		ImVec4* colors = style.Colors;

		// 主背景色：深灰偏黑
		colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);

		// 边框颜色：比背景稍亮
		colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.28f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		// 核心主题色：紫色
		ImVec4 accentColor = ImVec4(0.60f, 0.35f, 0.90f, 1.00f);       // 紫色常态
		ImVec4 accentActive = ImVec4(0.70f, 0.45f, 1.00f, 1.00f);      // 紫色按下
		ImVec4 accentHover = ImVec4(0.65f, 0.40f, 0.95f, 1.00f);       // 紫色悬停

		// 头部颜色 (CollapsingHeader等)
		colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = accentHover;
		colors[ImGuiCol_HeaderActive] = accentActive;

		// 按钮颜色
		colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = accentHover;
		colors[ImGuiCol_ButtonActive] = accentActive;

		// 关键控件颜色
		colors[ImGuiCol_CheckMark] = accentColor; // 勾选框的勾
		colors[ImGuiCol_SliderGrab] = accentColor; // 滑块的圆点
		colors[ImGuiCol_SliderGrabActive] = accentActive;

		// 文字颜色
		colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

		//滑块、输入框、复选框的背景
		//必须设置成深色，否则白色的字写在白色的背景上就看不见了
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f); // 深灰背景
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f); // 鼠标悬停稍亮
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f); // 按下时


		//标题颜色
		colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f); // 平时深色
		colors[ImGuiCol_HeaderHovered] = accentColor; // 悬停变紫
		colors[ImGuiCol_HeaderActive] = ImVec4(0.70f, 0.45f, 1.00f, 1.00f);
	}
	//初始化
	void Init() {
		auto& io = ImGui::GetIO();
		io.Fonts->AddFontFromFileTTF("C:\\WINDOWS\\FONTS\\DENG.TTF", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
	}
	//绘制总成
	void Draw() {
		//gui_debug();
		Canvas();
		if (gui::windowFlag::menu) Menu1();
		if (gui::windowFlag::menu && gui::windowFlag::EditColor) EditColor();
	}
	//旧菜单
	void Menu() {
		ImGui::Begin(u8"killaura");
		if (gui::windowFlag::click_lock) {
			ImGui::TextColored(color::暗红色, u8"菜单聚焦锁定！");
		}
		if (ct_Flag::isInit) {
			ImGui::TextColored(color::深绿色, u8"初始化成功");
			ImGui::Text(u8"人物名称:%s", ct_Player::name);
			ImGui::Text("x:%.1f", *ct_Player::x); ImGui::SameLine();
			ImGui::Text("y:%.1f", *ct_Player::y); ImGui::SameLine();
			ImGui::Text("z:%.1f", *ct_Player::z);
			ImGui::Text(u8"我的队伍:%d", *ct_Player::team);
			ImGui::Text(u8"局内人数:%d", *ct_var::gamePlayerNum);
		}
		else {
			ImGui::TextColored(color::暗红色, u8"初始化失败！");
		}

		//功能选区
		if (ImGui::CollapsingHeader(u8"功能")) {
			UI::Checkbox(u8"子弹追踪", &Cg_Func::H_BulletTrace);
			ImGui::SameLine();
			UI::Checkbox(u8"自定义开火延迟", &Cg_Func::H_ShootDelay); ImGui::SameLine();
			UI::Checkbox(u8"自动开火", &Cg_Func::E_AutoFire);

			UI::Checkbox(u8"无后坐力", &Cg_Func::H_NoRecoil); ImGui::SameLine();
			UI::Checkbox(u8"机器人禁止开枪", &Cg_Func::H_BotFire); ImGui::SameLine();
			UI::Checkbox(u8"机器人禁止移动", &Cg_Func::H_BotMove);

			UI::SliderInt(u8"射击延迟", &Cg_combat::shoot_delay, 0, 2000);
		}
		if (ImGui::CollapsingHeader(u8"移动")) {
			if (UI::Checkbox(u8"飞行", &Cg_Func::H_Fly)) {
				if (Cg_Func::H_Fly) Patch::Fly::CheckWall->Apply();
				else Patch::Fly::CheckWall->Restore();
			}
			ImGui::SameLine();
			UI::Checkbox(u8"应用重力值", &Cg_Func::H_Gravity); ImGui::SameLine();
			UI::Checkbox(u8"自动跳跃", &Cg_Func::H_AutoJump); ImGui::SameLine();

			UI::Checkbox(u8"加速", &Cg_Func::H_Speed);
			UI::Checkbox(u8"灵活控制", &Cg_Func::H_AbsoluteControl); ImGui::SameLine();
			UI::Checkbox(u8"无移动后摇", &Cg_Func::H_MoveRecovery);

			if (UI::Checkbox(u8"穿墙", &Cg_Func::H_Collide)) {
				if (Cg_Func::H_Collide) Patch::ThroughWall::Update();
				else Patch::ThroughWall::AllDone();
			}ImGui::SameLine();
			if (UI::RadioButton(u8"水平穿墙", (int*)&Cg_Move::ThrowMode, (int)ThroughWallMode::XY_through)) {
				if (Cg_Func::H_Collide) Patch::ThroughWall::Update();
			}ImGui::SameLine();
			if (UI::RadioButton(u8"纵向穿墙", (int*)&Cg_Move::ThrowMode, (int)ThroughWallMode::Z_through)) {
				if (Cg_Func::H_Collide) Patch::ThroughWall::Update();
			}ImGui::SameLine();
			if (UI::RadioButton(u8"全向穿墙", (int*)&Cg_Move::ThrowMode, (int)ThroughWallMode::ALL_through)) {
				if (Cg_Func::H_Collide) Patch::ThroughWall::Update();
			}

			UI::SliderFloat(u8"重力值", &Cg_Move::Gravity, -200, 200);
			UI::SliderFloat(u8"跳跃高度", &Cg_Move::High, 20, 100);
			UI::SliderFloat(u8"速度倍数", &Cg_Move::Speed, 1, 10);
			//ImGui::InputFloat(u8"速度自定义", &Cg_Move::Speed, 1.0f);
		}
		if (ImGui::CollapsingHeader(u8"AimBot")) {
			UI::Checkbox(u8"自瞄", &Cg_AimBot::Enable); ImGui::SameLine();
			UI::RadioButton(u8"平滑自瞄", (int*)&Cg_AimBot::mode, AimMode::smooth); ImGui::SameLine();
			UI::RadioButton(u8"强锁", (int*)&Cg_AimBot::mode, AimMode::lock);
			UI::SliderInt(u8"平滑度", &Cg_AimBot::smooth, 1, 30);
		}
		if (ImGui::CollapsingHeader(u8"Esp")) {
			UI::Checkbox(u8"显示方框", &Cg_Esp::show_box); ImGui::SameLine();
			UI::Checkbox(u8"显示名字", &Cg_Esp::show_name); ImGui::SameLine();
			UI::Checkbox(u8"显示血量", &Cg_Esp::show_blood); ImGui::SameLine();
			UI::Checkbox(u8"显示队伍", &Cg_Esp::show_team);

			UI::Checkbox(u8"显示敌人信息", &Cg_Esp::show_TargetInfo); ImGui::SameLine();
			UI::Checkbox(u8"显示子弹落点", &Cg_Esp::show_BulletImpact); ImGui::SameLine();
			UI::Checkbox(u8"显示实体数量", &Cg_Esp::show_PlayerNum);

			UI::Checkbox(u8"显示射线", &Cg_Esp::show_line); ImGui::SameLine();
			UI::Checkbox(u8"显示距离", &Cg_Esp::show_distance); ImGui::SameLine();
			UI::Checkbox(u8"显示开启功能", &Cg_Esp::show_FunsState);

			UI::ToggleButton(u8"可视化标注", &Cg_Esp::VisualJudgment);

			UI::RadioButton(u8"2D方框", (int*)&Cg_Esp::boxType, EspBoxType::rect2d); ImGui::SameLine();
			UI::RadioButton(u8"2D边框", (int*)&Cg_Esp::boxType, EspBoxType::edge2d); ImGui::SameLine();
			UI::RadioButton(u8"3D方框", (int*)&Cg_Esp::boxType, EspBoxType::threeD);
		}
		if (ImGui::CollapsingHeader(u8"娱乐功能")) {
			UI::Checkbox(u8"围绕目标旋转", &Cg_Func::H_Rotate);
			UI::SliderFloat(u8"跟随距离", &Cg_Recreation::setDistance, 1.0f, 10.0f);
			UI::SliderFloat(u8"旋转误差", &Cg_Recreation::setDeviation1, 1.0f, 5.0f);
			UI::SliderFloat(u8"外层吸附范围", &Cg_Recreation::setDeviation2, 0.0f, 10.0f);
		}
		if (ImGui::CollapsingHeader(u8"常规通用设置")) {
			UI::Checkbox(u8"过滤圈外实体", &Cg_general::filter_circle); ImGui::SameLine();
			UI::Checkbox(u8"过滤身后实体", &Cg_general::filter_inscreen); ImGui::SameLine();
			UI::Checkbox(u8"过滤不可见实体", &Cg_general::filter_invisible);

			UI::Checkbox(u8"过滤队友", &Cg_general::filter_teammate); ImGui::SameLine();
			UI::Checkbox(u8"过滤幽灵", &Cg_general::filter_ghost); ImGui::SameLine();
			UI::Checkbox(u8"过滤死人", &Cg_general::filter_dead);
		}
		if (ImGui::CollapsingHeader(u8"战斗相关配置")) {
			ImGui::Text(u8"索敌模式"); ImGui::SameLine();
			UI::RadioButton(u8"距离最近", (int*)&Cg_combat::CTMode, choiceMode::distance); ImGui::SameLine();
			UI::RadioButton(u8"准心最近", (int*)&Cg_combat::CTMode, choiceMode::sight);

			UI::SliderFloat(u8"过滤圈半径", &Cg_combat::circle_radius, 50.0f, 1000.0f);
		}
		if (ImGui::CollapsingHeader(u8"设置")) {
			UI::Checkbox(u8"打开调色板", &gui::windowFlag::EditColor); ImGui::SameLine();
			UI::Checkbox(u8"自动保存配置", &Config::AutoSave);
		}

		if (ImGui::Button(u8"保存配置")) {
			Config::SaveConfig();
		}
		ImGui::End();
	}
	//新菜单
	void Menu1() {
		//设置窗口固定大小
		ImGui::SetNextWindowSize(ImVec2(670, 450), ImGuiCond_Once);

		ImGui::Begin("Killaura V1.0", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

		// 记录当前选中的页面 (0主页	1战斗	 2移动	3视觉	4设置)
		static int currentTab = 0;
		//导航栏
		{
			// 创建一个子窗口作为侧边栏，宽度 130
			ImGui::BeginChild("Sidebar", ImVec2(140, 0), true);

			// 加上一点装饰文字
			ImGui::TextDisabled("Killaura V1.0");
			ImGui::Spacing();
			ImGui::Separator();

			// 绘制大按钮 (Tab)
			// 使用 ImVec2(-1, 45) 让按钮宽度占满侧边栏，高度 45 像素
			UI::TabButton(u8"主页", 0, &currentTab, ImVec2(-1, 45));
			ImGui::Spacing();
			UI::TabButton(u8"战斗功能", 1, &currentTab, ImVec2(-1, 45));
			ImGui::Spacing();
			UI::TabButton(u8"移动功能", 2, &currentTab, ImVec2(-1, 45));
			ImGui::Spacing();
			UI::TabButton(u8"透视&视觉", 3, &currentTab, ImVec2(-1, 45));
			ImGui::Spacing();
			UI::TabButton(u8"其他设置", 4, &currentTab, ImVec2(-1, 45));

			ImGui::EndChild();
		}

		ImGui::SameLine(); // 核心：让下一个元素在右边绘制，而不是下面

		// --- 右侧：内容区 (Content) ---
		{
			// 创建一个子窗口作为右侧容器，去除边框让它看起来和背景一体
			ImGui::BeginChild("Content", ImVec2(0, 0), false);

			// 给内容区加点内边距，不要紧贴着左边
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));

			// 根据 Tab 显示不同内容
			switch (currentTab) {
				//主页
			case 0:MenuPage::MainPage(); break;
				//战斗功能
			case 1:MenuPage::CombatPage(); break;
				//移动功能
			case 2:MenuPage::MovePage(); break;
				//透视&视觉
			case 3:MenuPage::EspPage(); break;
				//设置
			case 4:MenuPage::SetPage(); break;
			}

			ImGui::PopStyleVar(); // 恢复 Padding
			ImGui::EndChild();
		}

		ImGui::End();
	}
	//画布，绘制透视，文字，圈等
	void Canvas() {
		DrawEsp();//透视功能总成！
		DrawCircle();//绘制过滤圈
	}
	//调色板窗口
	void EditColor() {
		//设置一次窗口大小
		ImGui::SetNextWindowSize(ImVec2(350, 160), ImGuiCond_Once);

		//禁止折叠	禁止调整大小	隐藏标题栏
		ImGui::Begin("调色板", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		static int currentCombo = 0;//选择的选项下标
		const char* comboItems[] = {
			u8"透视颜色", u8"最佳目标颜色", u8"可见目标颜色",u8"不可见目标颜色",u8"过滤圈颜色",
			u8"信息窗口文字颜色",u8"信息窗口背景颜色",u8"功能名称颜色" };
		static ImColor* cur = &Cg_Color::Esp_color;
		static ColorMode* CurMode = &Cg_Color::Esp_color_mode;
		if (ImGui::Combo(u8"下拉选择", &currentCombo, comboItems, IM_ARRAYSIZE(comboItems))) {

			switch (currentCombo) {
			case 0:cur = &Cg_Color::Esp_color; CurMode = &Cg_Color::Esp_color_mode; break;//通用透视颜色
			case 1:cur = &Cg_Color::Esp_targetColor; CurMode = &Cg_Color::Esp_targetColor_mode; break;//最佳目标颜色
			case 2:cur = &Cg_Color::Esp_VisibleColor; CurMode = &Cg_Color::Esp_VisibleColor_mode; break;//可见目标颜色
			case 3:cur = &Cg_Color::Esp_InVisibleColor; CurMode = &Cg_Color::Esp_InVisibleColor_mode; break;//不可见目标颜色
			case 4:cur = &Cg_Color::Combat_CircleColor; CurMode = &Cg_Color::Combat_CircleColor_mode; break;//过滤圈颜色

				//信息窗口不提供彩虹模式了
			case 5:cur = &Cg_Color::Esp_infoTextColor; CurMode = nullptr; break;//文字颜色
			case 6:cur = &Cg_Color::Esp_infoBackColor; CurMode = nullptr; break;//背景颜色

			case 7:cur = &Cg_Color::Esp_FuncTextColor; CurMode = &Cg_Color::Esp_FuncTextColor_mode; break;//功能名称颜色

			default: cur = &Cg_Color::Esp_color;  CurMode = &Cg_Color::Esp_color_mode; currentCombo = 0; break;
			}

		}
		if (CurMode) {
			ImGui::RadioButton(u8"彩虹模式", (int*)CurMode, (int)ColorMode::Rainbow); ImGui::SameLine();
			ImGui::RadioButton(u8"自定义颜色", (int*)CurMode, (int)ColorMode::Customize);
		}
		ImGui::ColorEdit4(u8"自定义颜色", (float*)cur);
		ImGui::End();
	}
	//调试信息输出
	void Debug() {
		ImGui::Begin(u8"Debug");
		static bool AutoRotate = false;
		static uintptr_t ptr = 0;
		static int cycle = 35, time = 30, step = 2;
		if (ImGui::Button(u8"设立坐标点")) {
			Cg_Recreation::point = new Vector3(*ct_Player::x, *ct_Player::y, *ct_Player::z);
			ptr = (uintptr_t)Cg_Recreation::point;
			//Cg_Recreation::point->z -= 14.0f;
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"取消坐标点")) {
			if (ptr != 0 && ptr == (uintptr_t)Cg_Recreation::point) {
				Cg_Recreation::point = nullptr;
				delete (Vector3*)ptr;
				ptr = 0;
			}
		}
		if (ptr != 0 && ptr != (uintptr_t)Cg_Recreation::point) {
			delete (Vector3*)ptr;
			ptr = 0;
		}
		if (ImGui::Checkbox(u8"围绕目标旋转", &Cg_Func::H_Rotate)); ImGui::SameLine();
		if (ImGui::Checkbox(u8"自动旋转", &AutoRotate)) *ct_Player::AdvanceState = AutoRotate;
		if (ImGui::CollapsingHeader(u8"动点设置")) {
			UI::SliderInt(u8"前进次数", &cycle, 1, 100);
			ImGui::InputInt(u8"前进时间", &time, 1, 5);
			UI::SliderInt(u8"步进", &step, 1, 50);
		}
		if (ImGui::CollapsingHeader(u8"跟随范围设置")) {
			UI::SliderFloat(u8"跟随距离", &Cg_Recreation::setDistance, 1.0f, 10.0f);
			UI::SliderFloat(u8"旋转误差", &Cg_Recreation::setDeviation1, 1.0f, 5.0f);
			UI::SliderFloat(u8"加速跟随相对距离的偏差", &Cg_Recreation::setDeviation2, 1.0f, 10.0f);
		}
		if (Cg_Recreation::point) {
			auto draw = ImGui::GetBackgroundDrawList();
			Entity& best = EntityManager.LocalPlayer;
			Vector3& LocalPos = best.worldPos;
			if (ptr != 0 && ptr == (uintptr_t)Cg_Recreation::point) RectMove(*Cg_Recreation::point, cycle, time, step);
			Vector3 targetPos = *Cg_Recreation::point;
			float distance = (targetPos - LocalPos).Length2D() * 0.1f;
			string str = u8"距离" + to_string(distance) + u8"米";
			Vector3 screenPos;
			targetPos.z -= 14.0f;
			WorldToScreen(targetPos, screenPos, ct_var::mvpMatrix, Config::window_size.x, Config::window_size.y);
			draw->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), 3.0f, ImU32(color::彩虹));
			draw->AddText(ImVec2(screenPos.x, screenPos.y), ImU32(color::彩虹), str.c_str());
			ImColor col = color::鲜红色;
			DrawCircle3D(targetPos, col, 32, (Cg_Recreation::setDistance + Cg_Recreation::setDeviation2) * 10);//加速跟随范围
			ImGui::Text(u8"距离:%.2f米", distance);
		}

		ImGui::End();
	}
}

//我们对ImGui函数的封装
//封装多一层主要是为了我们好进行标志位的更改
namespace UI {
	//封装版本
	bool Checkbox(const char* label, bool* v) {
		if (ImGui::Checkbox(label, v)) {

			Config::FuncChange = true;

			return true;
		}
		return false;
	}
	//封装版本
	bool RadioButton(const char* label, int* v, int v_button) {
		if (ImGui::RadioButton(label, v, v_button)) {

			Config::FuncChange = true;

			return true;
		}
		return false;
	}
	//封装版本
	bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
		if (ImGui::SliderFloat(label, v, v_min, v_max, format, flags)) {

			Config::FuncChange = true;

			return true;
		}
		return false;
	}
	//封装版本
	bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
		if (ImGui::SliderInt(label, v, v_min, v_max, format, flags)) {

			Config::FuncChange = true;

			return true;
		}
		return false;
	}
	//圆角checkbox控件
	bool ToggleButton(const char* str_id, bool* v) {
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		float height = ImGui::GetFrameHeight();//获得ImGui标准控件的高度	为了统一
		float width = height * 1.55f;
		float radius = height * 0.50f;
		bool ret = false;

		// 透明按钮覆盖在上面，处理点击逻辑
		if (ImGui::InvisibleButton(str_id, ImVec2(width, height))) {
			*v = !*v;
			Config::FuncChange = true;
			ret = true;
		}
			

		ImU32 col_bg;
		if (ImGui::IsItemHovered())
			col_bg = *v ? IM_COL32(0, 220, 110, 255) : IM_COL32(110, 110, 110, 255); // 悬停变色
		else
			col_bg = *v ? IM_COL32(0, 200, 100, 255) : IM_COL32(80, 80, 80, 255); // 开启/关闭颜色

		// 画圆角矩形背景
		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);

		// 画圆球
		draw_list->AddCircleFilled(
			ImVec2(*v ? (p.x + width - radius) : (p.x + radius), p.y + radius),
			radius - 1.5f,
			IM_COL32(255, 255, 255, 255)
		);

		// 显示文字标签 (在开关右边)
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding(); // 文字垂直对齐
		ImGui::Text("%s", str_id);

		return ret;
	}
	//菜单功能板块切换按钮
	void TabButton(const char* label, int index, int* selected_index, ImVec2 size) {
		//获取当前的主题色
		ImVec4 accentColor = ImGui::GetStyle().Colors[ImGuiCol_CheckMark];//checkbox√的颜色

		//判断是否被选中
		bool is_selected = (*selected_index == index);

		//如果选中了，就把按钮颜色改成主题色 (PushStyleColor)
		if (is_selected) {
			// 按钮常态颜色
			ImGui::PushStyleColor(ImGuiCol_Button, accentColor);
			// 按钮悬停颜色 (稍微亮一点)
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(accentColor.x + 0.1f, accentColor.y + 0.1f, accentColor.z + 0.1f, 1.0f));
			// 按钮按下颜色
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, accentColor);
		}
		else {
			//没选中，透明背景
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			// 没选中时的悬停颜色 (深灰)
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
		}

		//绘制按钮
		if (ImGui::Button(label, size)) {
			*selected_index = index; // 点击后更新选中索引
		}

		//还原颜色 (PopStyleColor)
		// 我们刚才 Push 了 3 个颜色，所以要 Pop 3 个
		ImGui::PopStyleColor(3);
	}
	//网址文本
	void TextURL(const char* name, const char* url, bool sameLineBefore)
	{
		if (sameLineBefore) ImGui::SameLine();
		//ImVec4(0.4f, 0.7f, 1.0f, 1.0f)

		//浅蓝色文字
		ImGui::PushStyleColor(ImGuiCol_Text, color::霓虹蓝);
		ImGui::Text(name);
		ImGui::PopStyleColor();

		//鼠标悬停时的交互
		if (ImGui::IsItemHovered())
		{
			// 鼠标变成手型
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

			//简单的Tooltip提示URL
			ImGui::BeginTooltip();
			ImGui::Text("%s", url);
			ImGui::EndTooltip();

			//绘制下划线，表示这是一个链接
			ImVec2 min = ImGui::GetItemRectMin();//控件左上角
			ImVec2 max = ImGui::GetItemRectMax();//控件右下角
			min.y = max.y; //左上->左下		在底部划线
			//ImGui::GetColorU32(ImVec4(0.4f, 0.7f, 1.0f, 1.0f))
			ImGui::GetWindowDrawList()->AddLine(min, max,ImColor(color::霓虹蓝));
			ImGui::GetBackgroundDrawList(); ImGui::GetForegroundDrawList();
				
		}

		//如果点击
		if (ImGui::IsItemClicked())
		{
			//打开windows默认浏览器
			ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
		}
	}
}

//ImGui扩展
namespace ImGui {
	void BeginGroupPanel(const char* name, const ImVec2& size)
	{
		ImGui::BeginGroup();

		auto cursorPos = ImGui::GetCursorScreenPos();
		auto itemSpacing = ImGui::GetStyle().ItemSpacing;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();
		ImGui::BeginGroup();

		ImVec2 effectiveSize = size;
		if (size.x < 0.0f)
			effectiveSize.x = ImGui::GetContentRegionAvail().x;
		else
			effectiveSize.x = size.x;
		ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::BeginGroup();
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::TextUnformatted(name);
		auto labelMin = ImGui::GetItemRectMin();
		auto labelMax = ImGui::GetItemRectMax();
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
		ImGui::BeginGroup();

		//ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

		ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x -= frameHeight;

		auto itemWidth = ImGui::CalcItemWidth();
		ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

		s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
	}

	void EndGroupPanel()
	{
		ImGui::PopItemWidth();

		auto itemSpacing = ImGui::GetStyle().ItemSpacing;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();

		ImGui::EndGroup();

		//ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255, 0, 64), 4.0f);

		ImGui::EndGroup();

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

		ImGui::EndGroup();

		auto itemMin = ImGui::GetItemRectMin();
		auto itemMax = ImGui::GetItemRectMax();
		//ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

		auto labelRect = s_GroupPanelLabelStack.back();
		s_GroupPanelLabelStack.pop_back();

		ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
		ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));
		labelRect.Min.x -= itemSpacing.x;
		labelRect.Max.x += itemSpacing.x;
		for (int i = 0; i < 4; ++i)
		{
			switch (i)
			{
				// left half-plane
			case 0: ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true); break;
				// right half-plane
			case 1: ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true); break;
				// top
			case 2: ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true); break;
				// bottom
			case 3: ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true); break;
			}

			ImGui::GetWindowDrawList()->AddRect(
				frameRect.Min, frameRect.Max,
				ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
				halfFrame.x);

			ImGui::PopClipRect();
		}

		ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x += frameHeight;

		ImGui::Dummy(ImVec2(0.0f, 0.0f));

		ImGui::EndGroup();
	}
}

void RectMove(Vector3& point, int cycle, int time, int step) {
	static int last = *ct_var::lastmillis;
	static int cnt = 0, choice = 0;
	if (*ct_var::lastmillis - last > time) {
		if (cnt >= cycle) {
			if (choice == 3) choice = 0;
			else choice++;
			cnt = 0;
		}
		switch (choice) {
		case 0:point.x += step; ++cnt; break;
		case 1:point.y += step; ++cnt; break;
		case 2:point.x -= step; ++cnt; break;
		case 3:point.y -= step; ++cnt; break;
		default:break;
		}
		last = *ct_var::lastmillis;
	}

}