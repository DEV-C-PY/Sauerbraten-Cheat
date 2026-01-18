#pragma once
#pragma once
#include <optional>
#include <minwindef.h>
#include "../vec3/Vector.h"
#include "../imgui_gl/imgui.h"
#include "../Manager/ConfigManager.h"

//返回点是否可见
//1.世界坐标	2.输出屏幕2D坐标	3.mvp矩阵	4.宽度	5.长度
bool WorldToScreen(Vector3 pWorldPos, Vector3& pScreenPos, float* pMatrixPtr, const FLOAT pWinWidth, const FLOAT pWinHeight);

//透视功能总成！
void DrawEsp();
//绘制过滤圈
void DrawCircle();

enum textEsp {
	blood, name, 距离,队伍	//直接中文，防止与ConfigManager.h里的distance冲突
};
//绘制功能子模块，外界调用DrawEsp即可！
//绘制敌人的方框
//1.实体  2.颜色    3.方框类型
void DrawBox(const Entity& player, ImColor& color,EspBoxType boxType);
//从游戏顶部中间位置绘制一条指向敌人的线段
//1.实体	2.颜色
void DrawLine(const Entity& player, ImColor& color);
//在方框旁边绘制文字
//1.实体  2.颜色    3.绘制的行数     4.绘制的属性
void DrawBoxText(const Entity& player, ImColor& color, int& line, textEsp sw);
//绘制过滤圈
void DrawCircle();
// 绘制圆角窗口信息
// 1.窗口左上角位置 2.信息内容 3.背景颜色 4.文字颜色   5.菜单开启状态（是否允许拖拽）
void DrawInfo(ImVec2& ScreenPos, const std::string& info, ImColor& BackGroundColor, ImColor& TextColor,bool is_menu_open);
//绘制子弹\射线落点
//1.世界坐标	2.颜色	3.大小
void DrawBulletImpact(const Vector3& worldPos, ImColor& color , int size = 2.5f);
//绘制3D圆形
//1.世界坐标	2.颜色	3.精度，线段	4.圆形半径	5.线段粗细
void DrawCircle3D(const Vector3& worldPos, ImColor& color, int precision, float radius, float thin = 2.0f);
//绘制局内人数情况
void DrawPlayerNum();
//绘制功能名称以及拖动逻辑
//1.菜单开启状态（是否允许拖拽）  2.颜色    3.字体大小（默认20.0f）
void DrawFuncState(bool is_menu_open, ImColor col, float font_size = 20.0f);
//拖拽组件函数
//1.组件的左上角坐标 (会被修改) 2.组件的宽高    3.是否允许拖拽（菜单开启状态）   4.静态标志位，用于记录当前组件是否正在被拖拽
bool HandleComponentDrag(ImVec2& pos, ImVec2 size, bool is_active, bool& is_dragging_state);