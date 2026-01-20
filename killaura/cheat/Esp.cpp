#include <cstdint>
#include <optional>
#include <Windows.h>
#include <cmath> 
#include <string>
#include "../vec3/Vector.h"
#include "EntityCache.h"
#include "../imgui_gl/imgui.h"
#include "../Manager/ConfigManager.h"
#include "../cheat/Esp.h"
#include "../cheat/cheat.h"
#include "../gui/gui.h"

// 辅助数学函数：角度转弧度
#define M_PI 3.14159265358979323846f
#define DEG2RAD(x) ((x) * (M_PI / 180.0f))

bool WorldToScreen(
    Vector3 pWorldPos,
    Vector3& pScreenPos,
    float* pMatrixPtr,
    const FLOAT pWinWidth,
    const FLOAT pWinHeight
) {
    // 计算齐次坐标的w分量
    float w = pWorldPos.x * pMatrixPtr[3] +
        pWorldPos.y * pMatrixPtr[7] +
        pWorldPos.z * pMatrixPtr[11] +
        pMatrixPtr[15];

    // 如果w值太小，表示点在摄像机后面，不可见
    if (w < 0.001f) {
        return false;
    }

    // 计算投影后的x和y分量
    float x = pWorldPos.x * pMatrixPtr[0] +
        pWorldPos.y * pMatrixPtr[4] +
        pWorldPos.z * pMatrixPtr[8] +
        pMatrixPtr[12];

    float y = pWorldPos.x * pMatrixPtr[1] +
        pWorldPos.y * pMatrixPtr[5] +
        pWorldPos.z * pMatrixPtr[9] +
        pMatrixPtr[13];

    // 归一化到NDC空间[-1, 1]
    float nx = x / w;
    float ny = y / w;

    // 转换到屏幕空间
    float windowCenterX = pWinWidth / 2.0f;
    float windowCenterY = pWinHeight / 2.0f;

    // 设置屏幕坐标，z分量设为0（2D屏幕坐标）
    pScreenPos.x = windowCenterX + (windowCenterX * nx);
    pScreenPos.y = windowCenterY - (windowCenterY * ny);  // Y轴翻转
    pScreenPos.z = 0.0f;

    return true;
}
//获得实体颜色
ImColor& GetEntityColor(Entity& player) {
    //需要处理普通，可视化，最佳目标的颜色
    ImColor* col;
    
    //最佳目标优先级最高
    if (player.MyIndex() == EntityManager.bestTarget.MyIndex()) {
        if (Cg_Color::Esp_targetColor_mode == ColorMode::Customize) col = &Cg_Color::Esp_targetColor;
        else col = &color::彩虹;
        return *col;
    }
    //可视化标注
    if (Cg_Esp::VisualJudgment) {
        if (player.IsVisible == true) {
            if (Cg_Color::Esp_VisibleColor_mode == ColorMode::Customize) col = &Cg_Color::Esp_VisibleColor;
            else col = &color::彩虹;
        }
        else {
            if (Cg_Color::Esp_InVisibleColor_mode == ColorMode::Customize) col = &Cg_Color::Esp_InVisibleColor;
            else col = &color::彩虹;
        }
        return *col;
    }
    //普通
    if (Cg_Color::Esp_color_mode == ColorMode::Customize) col = &Cg_Color::Esp_color;
    else col = &color::彩虹;

    return *col;
}
//获得过滤圈颜色
ImColor& GetCircleColor() {
    if (Cg_Color::Combat_CircleColor_mode == ColorMode::Customize) return Cg_Color::Combat_CircleColor;
    else return color::彩虹;
}
void DrawEsp() {
    for (int i = 0; i < EntityManager.EnemyNum; ++i) {
        Entity& player = EntityManager[i];

        if (Cg_general::filter_teammate && player.team == EntityManager.LocalPlayer.team) continue;//过滤队友
        if (Cg_general::filter_ghost && player.IsGhost()) continue;//过滤幽灵
        if (Cg_general::filter_dead && !player.IsAlive()) continue;//过滤死人
        if (!player.IsInScreen) continue;//敌人不可见，跳过

        ImColor& curColor = GetEntityColor(player);
        
        int line = 0;//文字显示的行数
        if (Cg_Esp::show_box) DrawBox(player, curColor,Cg_Esp::boxType);
        if (Cg_Esp::show_line) DrawLine(player, curColor);
        if (Cg_Esp::show_blood) DrawBoxText(player, curColor,line, textEsp::blood);
        if (Cg_Esp::show_name) DrawBoxText(player, curColor, line, textEsp::name);
        if (Cg_Esp::show_distance) DrawBoxText(player, curColor, line, textEsp::距离);
        if (Cg_Esp::show_team) DrawBoxText(player, curColor, line, textEsp::队伍);
    }
    //绘制环绕圈
    if (Cg_Func::H_Rotate && Cg_Recreation::point) {
        Vector3 feet = *Cg_Recreation::point;
        feet.z -= 14.0f;
        if (EntityManager.bestTarget.isInit())
        {
            ImColor& col = GetEntityColor(EntityManager.bestTarget);
            DrawCircle3D(feet, col, 32, Cg_Recreation::setDistance * 10);//旋转范围
            DrawCircle3D(feet, col, 32, (Cg_Recreation::setDistance + Cg_Recreation::setDeviation2) * 10);//加速跟随范围
        }
    }
	//绘制敌人信息窗口
    if (Cg_Esp::show_TargetInfo && (EntityManager.bestTarget.isInit() || gui::windowFlag::menu)) {
		Cg_Esp::infoText = u8"最佳目标信息\n";
        //获得最佳目标，如果没有最佳目标就显示自己的信息
        Entity& best = EntityManager.bestTarget.isInit() ? EntityManager.bestTarget : EntityManager.LocalPlayer;
		Cg_Esp::infoText += u8"名字:" + std::string(best.name) + "\n";
		Cg_Esp::infoText += u8"血量:" + std::to_string(best.blood) + "\n";
		float cut = static_cast<float>(best.distance);
        Cg_Esp::infoText += u8"距离:";
		Cg_Esp::infoText += std::to_string(cut) + u8"米\n";
		Cg_Esp::infoText += u8"队伍:" + std::to_string(best.team) + "\n";
		DrawInfo(Cg_Esp::InfoPos, Cg_Esp::infoText, Cg_Color::Esp_infoBackColor , Cg_Color::Esp_infoTextColor,gui::windowFlag::menu);
    }
    //绘制子弹落点
    if (Cg_Esp::show_BulletImpact) {
        Vector3 worldpos(*ct_var::worldpos_x, *ct_var::worldpos_y, *ct_var::worldpos_z);
        DrawBulletImpact(worldpos, Cg_Color::Esp_color);
    }
    //绘制局内人数情况
    if (Cg_Esp::show_PlayerNum) {
        DrawPlayerNum();
    }
    //绘制功能开启情况
    if (Cg_Esp::show_FunsState) {
        if (Cg_Esp::ActiveFuncList.empty()) return;

        if (Cg_Color::Esp_FuncTextColor_mode == ColorMode::Rainbow) DrawFuncState(gui::windowFlag::menu, color::彩虹);
        else DrawFuncState(gui::windowFlag::menu, Cg_Color::Esp_FuncTextColor);
    }
}

//绘制敌人的方框
//1.实体  2.颜色    3.方框类型
void DrawBox(const Entity& player, ImColor& color, EspBoxType boxType) {
    // 基础2D数据计算
    const float height = ::abs(player.ScreenHeadPos.y - player.ScreenFeetPos.y) * 1.25f;//方框高度
    const float width = height / 2.f;//方框高度
    //方框左上角坐标 = 头部屏幕坐标 - 宽度 / 2
    const float x = player.ScreenHeadPos.x - (width / 2.f);
    const float y = player.ScreenHeadPos.y - (width / 2.5f); // 微调头部位置

    //屏幕尺寸
    const float screenW = Config::window_size.x;
    const float screenH = Config::window_size.y;
    //MVP矩阵指针
    static float* mvpMatrixPtr = ct_var::mvpMatrix;

    //画笔
    auto drawList = ImGui::GetBackgroundDrawList();
    float thickness = 2.0f; // 线条粗细

    switch (boxType) {
    case EspBoxType::rect2d: {
        // 完整2D矩形
        drawList->AddRect(
            ImVec2(x, y),
            ImVec2(x + width, y + height),
            color,
            0.0f, 0, thickness
        );
        break;
    }

    case EspBoxType::edge2d: {
        //2D四角方框
        //分别绘制四角的水平和垂直短线
        float lineW = width / 4.0f; // 短线长度，设为宽度的1/4
        float lineH = width / 4.0f; // 垂直短线长度，通常也取宽度的比例以保持对称，或者取height/4

        // 左上角 
        drawList->AddLine(ImVec2(x, y), ImVec2(x, y + lineH), color, thickness);
        drawList->AddLine(ImVec2(x, y), ImVec2(x + lineW, y), color, thickness);

        // 右上角 
        drawList->AddLine(ImVec2(x + width, y), ImVec2(x + width, y + lineH), color, thickness);
        drawList->AddLine(ImVec2(x + width, y), ImVec2(x + width - lineW, y), color, thickness);

        // 左下角 
        drawList->AddLine(ImVec2(x, y + height), ImVec2(x, y + height - lineH), color, thickness);
        drawList->AddLine(ImVec2(x, y + height), ImVec2(x + lineW, y + height), color, thickness);

        // 右下角 
        drawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width, y + height - lineH), color, thickness);
        drawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width - lineW, y + height), color, thickness);
        break;
    }

    case EspBoxType::threeD: {
        //人物基础数据
        Vector3 headPos = player.worldPos; 
        Vector3 feetPos = headPos;
        headPos.z += 2.0f; //微调头部坐标，让方框更长
		feetPos.z -= 15.0f; // 脚部位置

        //半径，半宽
        float boxHalfSize = 3.8f; 

        //计算方框旋转角度
        float yawRad = DEG2RAD(player.Yaw);
        float cosY = cos(yawRad);
        float sinY = sin(yawRad);

        //4个角的偏移量
        float dx = boxHalfSize;
        float dy = boxHalfSize;

        // 预先计算旋转后的四个向量 (x, y)
        // 公式: x' = x*cos - y*sin, y' = x*sin + y*cos
        Vector3 corners[4];

        //右前 (+x, +y)
        corners[0].x = dx * cosY - dy * sinY;
        corners[0].y = dx * sinY + dy * cosY;

        //左前 (+x, -y)
        corners[1].x = dx * cosY - (-dy) * sinY;
        corners[1].y = dx * sinY + (-dy) * cosY;

        //左后 (-x, -y)
        corners[2].x = (-dx) * cosY - (-dy) * sinY;
        corners[2].y = (-dx) * sinY + (-dy) * cosY;

        //右后 (-x, +y)
        corners[3].x = (-dx) * cosY - dy * sinY;
        corners[3].y = (-dx) * sinY + dy * cosY;

        //转换8个世界坐标为屏幕坐标
        //并且记录8个点是否可见
        ImVec2 screenPoints[8];//0-3:底部四个点  4-7:顶部四个点
        bool   pointVisible[8];// 记录每个点是否可见

        for (int i = 0; i < 4; i++) {
            // 底部点
            Vector3 worldPtBottom = { feetPos.x + corners[i].x, feetPos.y + corners[i].y, feetPos.z };
            Vector3 screenVecBottom;
            pointVisible[i] = WorldToScreen(worldPtBottom, screenVecBottom, mvpMatrixPtr, screenW, screenH);
            screenPoints[i] = ImVec2(screenVecBottom.x, screenVecBottom.y);

            // 顶部点 (使用 headPos 的 Z 轴)
            Vector3 worldPtTop = { headPos.x + corners[i].x, headPos.y + corners[i].y, headPos.z };
            Vector3 screenVecTop;
            pointVisible[i + 4] = WorldToScreen(worldPtTop, screenVecTop, mvpMatrixPtr, screenW, screenH);
            screenPoints[i + 4] = ImVec2(screenVecTop.x, screenVecTop.y);
        }

        //连接索引
        int lines[12][2] = {
            {0,1}, {1,2}, {2,3}, {3,0}, // 底部矩形
            {4,5}, {5,6}, {6,7}, {7,4}, // 顶部矩形
            {0,4}, {1,5}, {2,6}, {3,7}  // 4根柱子
        };

        for (int i = 0; i < 12; i++) {
            int p1 = lines[i][0];
            int p2 = lines[i][1];

            // 只有当两个点都在屏幕内（或至少W2S计算成功）时才绘制
            if (pointVisible[p1] && pointVisible[p2]) {
                drawList->AddLine(screenPoints[p1], screenPoints[p2], color, thickness);
            }
        }
        break;
    }

    default:
        break;
    }
}
//从游戏顶部中间位置绘制一条指向敌人的线段
//1.实体	2.颜色
void DrawLine(const Entity& player, ImColor& color) {
    auto draw = ImGui::GetBackgroundDrawList();
    ImVec2 p1(Config::window_size.x / 2, 0);
    ImVec2 p2(player.ScreenHeadPos.x, player.ScreenHeadPos.y);
    draw->AddLine(p1, p2, color, 2.0f);
}
//在方框旁边绘制文字
//1.实体  2.颜色    3.绘制的行数     4.绘制的属性
void DrawBoxText(const Entity& player, ImColor& color,int& line,textEsp sw) {
    const float height = ::abs(player.ScreenHeadPos.y - player.ScreenFeetPos.y) * 1.25f;
    const float width = height / 2.f;
    //在方框的右上角开始绘制文字
    ImVec2 textPos = ImVec2(player.ScreenHeadPos.x + width / 2, player.ScreenHeadPos.y + line * 20.0f);//计算行数
    std::string text;
    switch (sw) {
        case textEsp::blood: {
            text = u8"血量:" + std::to_string(player.blood);
            break;
        }
        case textEsp::name:{
            text = u8"名字:";
            text += player.name;
            break;
        }
        case textEsp::距离:{
            int cut = static_cast<int>(player.distance);
            text = u8"距离:";
            text += std::to_string(cut) + u8"米";
            break;
        }
        case textEsp::队伍: {
            text = u8"队伍:";
			text += std::to_string(player.team);
            break;
        }
    }
    line += 1;
    ImGui::GetBackgroundDrawList()->AddText(textPos, color, text.c_str());
}

//绘制过滤圈
void DrawCircle() {
    if (!Cg_general::filter_circle) return;//未开启过滤功能，直接退出
    const float screenW = Config::window_size.x;
    const float screenH = Config::window_size.y;
    ImVec2 pos(screenW / 2, screenH / 2);
    ImGui::GetBackgroundDrawList()->AddCircle(pos, Cg_combat::circle_radius, GetCircleColor());
}

// 绘制圆角窗口信息
// 1.窗口左上角位置 2.信息内容 3.背景颜色 4.文字颜色   5.菜单开启状态（是否允许拖拽）
void DrawInfo(ImVec2& ScreenPos, const std::string& info, ImColor& BackGroundColor, ImColor& TextColor,bool is_menu_open) {
    auto drawList = ImGui::GetBackgroundDrawList();

    // --- 1. 布局计算 ---
    // 设定内边距 (Padding)，让文字不要贴着框的边缘
    const float paddingX = 8.0f;
    const float paddingY = 6.0f;

    // 计算文字的物理大小
    ImVec2 textSize = ImGui::CalcTextSize(info.c_str());

    // 根据文字大小自动算出矩形的大小
    ImVec2 rectSize = ImVec2(textSize.x + paddingX * 2, textSize.y + paddingY * 2);

    // 计算矩形的 左上角(Min) 和 右下角(Max)
    ImVec2 rectMin = ScreenPos;
    ImVec2 rectMax = ImVec2(rectMin.x + rectSize.x, rectMin.y + rectSize.y);

    static bool isDraggingInfo = false;
    //拖拽
    if (HandleComponentDrag(ScreenPos, rectSize, is_menu_open, isDraggingInfo)) Config::FuncChange = true;//触发自动保存

    //圆角
    const float rounding = 6.0f; // 圆角半径，值越大越圆

    //阴影
    ImColor shadowColor = IM_COL32(0, 0, 0, 80); // 黑色，Alpha 80
    drawList->AddRectFilled(
        ImVec2(rectMin.x + 2, rectMin.y + 2),
        ImVec2(rectMax.x + 2, rectMax.y + 2),
        shadowColor,
        rounding
    );

    //主背景
    drawList->AddRectFilled(rectMin, rectMax, BackGroundColor, rounding);

    //描边
    ImColor borderColor = IM_COL32(255, 255, 255, 128);
    drawList->AddRect(rectMin, rectMax, borderColor, rounding, 0, 1.0f);

    //绘制文字
    ImVec2 textPos = ImVec2(rectMin.x + paddingX, rectMin.y + paddingY);
    drawList->AddText(textPos, TextColor, info.c_str());
}

//绘制子弹\射线落点
//1.世界坐标	2.颜色	3.大小
void DrawBulletImpact(const Vector3& worldPos, ImColor& color, int size) {
    // 获取屏幕尺寸 (用于W2S)
    const float screenW = Config::window_size.x;
    const float screenH = Config::window_size.y;
    // 获取MVP矩阵指针
    static float* mvpMatrixPtr = ct_var::mvpMatrix;
    Vector3 ScreenDot;//点在屏幕的坐标
    WorldToScreen(worldPos, ScreenDot, mvpMatrixPtr, screenW, screenH);
    ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(ScreenDot.x, ScreenDot.y), size, ImColor(color::暗红色));
}

//绘制3D圆形
//1.世界坐标	2.颜色	3.精度，线段	4.圆形半径	5.线段粗细
void DrawCircle3D(const Vector3& worldPos, ImColor& color, int precision, float radius, float thin)
{
    float* pMatrix = ct_var::mvpMatrix; // [你的全局MVP矩阵数组指针]
    float   winWidth = (float)Config::window_size.x;  // [你的全局屏幕宽度]
    float   winHeight = (float)Config::window_size.y; // [你的全局屏幕高度]

    // 1. 性能优化：如果半径太小或者是无效颜色，直接不画
    if (radius <= 0.0f || color.Value.w <= 0.0f) return;

    // 2. 准备容器存储转换后的屏幕坐标点
    std::vector<ImVec2> points;
    // 预分配内存，避免循环中频繁 realloc
    points.reserve(precision);

    // 3. 计算步长 (2PI / 精度)
    const float step = 6.28318530718f / (float)precision;

    // 4. 循环生成 3D 点并转换
    for (int i = 0; i < precision; ++i)
    {
        float theta = step * (float)i; // 当前角度

        // [核心数学]
        // Cube2 坐标系：Z是高，XY是地面
        // 如果是 Unity/DirectX 游戏 (Y是高)，则要把 sin 改到 z 轴
        Vector3 currentWorldPos;
        currentWorldPos.x = worldPos.x + radius * std::cos(theta);
        currentWorldPos.y = worldPos.y + radius * std::sin(theta);
        currentWorldPos.z = worldPos.z; // 高度不变，保持在脚下

        Vector3 screenPos3D; // W2S 的输出结果

        // 5. 调用你的 W2S 函数
        if (WorldToScreen(currentWorldPos, screenPos3D, pMatrix, winWidth, winHeight))
        {
            // 转换成功，存入点集
            // 注意：W2S 输出的是 Vector3，ImGui 需要 ImVec2，取 x, y 即可
            Vector3 temp = *(Vector3*)(ct_var::worldpos_x);
            LandingPointCal(currentWorldPos);
            Vector3 rayStart(*ct_Player::x, *ct_Player::y, *ct_Player::z);//1.射线起点
            Vector3 rayEnd(*ct_var::worldpos_x, *ct_var::worldpos_y, *ct_var::worldpos_z);//2.射线落点
            Vector3 playerFeet(currentWorldPos);//3.实体脚部坐标
            Vector3 playerHead = playerFeet;//4.实体头部坐标
            playerHead.z += 2.0f;
            float hit_range = 4.1f;
            static float* outVal = new float{};//6.负责接收函数输出的值，没研究出来含义，无用
            //射线检测，返回可见状态
            if (func::IntersectRay(&rayStart, &rayEnd, &playerFeet, &playerHead, hit_range, outVal)) {
                points.push_back(ImVec2(screenPos3D.x, screenPos3D.y));
            }
            *(Vector3*)(ct_var::worldpos_x) = temp;
        }
        else
        {
            // [进阶处理]
            // 如果点在屏幕后面（W2S失败），直接忽略会导致连线出现“穿屏”错乱。
            // 简单的画圆通常忽略这个问题，或者你可以选择在这里截断 Polyline。
            // 为了代码简洁，这里保持默认行为（不添加该点）。
        }
    }

    // 6. 提交绘制
    // 只有当有效的点超过 2 个才能构成多边形
    if (points.size() > 2)
    {
        ImGui::GetBackgroundDrawList()->AddPolyline(
            points.data(),      // 点数组指针
            (int)points.size(), // 点数量
            color,              // 颜色 (ImColor 会自动转 ImU32)
            true,               // closed = true (自动闭合首尾，形成圆)
            thin                // 线宽
        );
    }
}

//绘制局内人数情况
void DrawPlayerNum() {
    auto draw = ImGui::GetBackgroundDrawList();
    ImVec2 WindowSize = Config::window_size;
    static ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];
    //因为EnemyNum是敌人数量，我们计算局内人数时需要加上自己
    std::string PlayerNum = u8"局内人数:" + std::to_string(EntityManager.EnemyNum + 1);
    std::string VisibleNum = u8"可见人数:" + std::to_string(EntityManager.VisibleEntityNum);
    WindowSize.x *= 0.5;
    WindowSize.x -= 120.0f;
    WindowSize.y *= 0.13;
    draw->AddText(pFont, 20.0f, WindowSize, Cg_Color::Esp_color, PlayerNum.c_str());
    WindowSize.x += 240.0f;
    draw->AddText(pFont, 20.0f, WindowSize, Cg_Color::Esp_VisibleColor, VisibleNum.c_str());
}

//绘制功能名称
//1.菜单开启状态（是否允许拖拽）  2.颜色    3.字体大小
void DrawFuncState(bool is_menu_open, ImColor col, float font_size) {
    // 引用简化
    std::vector<std::string>& list = Cg_Esp::ActiveFuncList;
    ImVec2& pos = Cg_Esp::FuncPos;
    AlignMode& align = Cg_Esp::AMode;

    if (list.empty()) return;

    // 准备资源
    ImFont* pFont = ImGui::GetFont();
    auto drawList = ImGui::GetBackgroundDrawList();
    float gap = 3.0f;

    // --- 计算包围盒尺寸 ---
    // 宽 (最长的一行)
    float longestWidth = pFont->CalcTextSizeA(font_size, FLT_MAX, 0.0f, list[0].c_str()).x;
    // 高 (总行高)
    float totalHeight = list.size() * (font_size + gap);
    ImVec2 rectSize = { longestWidth, totalHeight };

    //控件独立的拖拽记忆
    static bool isDraggingFunc = false;

    // 调用通用拖拽函数
    if (HandleComponentDrag(pos, rectSize, is_menu_open, isDraggingFunc)) {
        //只有在拖拽时，才进行对齐方式的重新判断
        ImGuiIO& io = ImGui::GetIO();

        // 计算组件中心 X 坐标
        float centerX = pos.x + (longestWidth / 2.0f);
        // 屏幕中心 X 坐标
        float screenCenterX = io.DisplaySize.x / 2.0f;

        //中心点在屏幕右侧 -> 右对齐，否则 -> 左对齐
        align = (centerX > screenCenterX) ? AlignMode::RIGHT : AlignMode::LEFT;

        //触发自动保存
        Config::FuncChange = true;
    }

    //绘制文字
    float currentY = pos.y;

    for (const auto& str : list) {
        float actualWidth = pFont->CalcTextSizeA(font_size, FLT_MAX, 0.0f, str.c_str()).x;
        float drawX = pos.x;

        if (align == AlignMode::RIGHT) {
            // 右对齐计算
            drawX = pos.x + longestWidth - actualWidth;
        }
        else {
            // 左对齐计算
            drawX = pos.x;
        }

        // 上下左右画黑色（描边）
        drawList->AddText(pFont, font_size, { drawX - 1, currentY }, ImColor(0, 0, 0, 255), str.c_str());
        drawList->AddText(pFont, font_size, { drawX + 1, currentY }, ImColor(0, 0, 0, 255), str.c_str());
        drawList->AddText(pFont, font_size, { drawX, currentY - 1 }, ImColor(0, 0, 0, 255), str.c_str());
        drawList->AddText(pFont, font_size, { drawX, currentY + 1 }, ImColor(0, 0, 0, 255), str.c_str());
        //实际绘制文字
        //drawList->AddText(pFont, font_size, { drawX, currentY }, col, str.c_str());

        //加粗绘制字体（循环三遍），观感比上面的方案好
        for (int i = 0; i < 3; ++i) {
            drawList->AddText(pFont, font_size, { drawX, currentY }, col, str.c_str());
        }

        currentY += font_size + gap;
    }
}

//拖拽组件函数
//1.组件的左上角坐标 (会被修改) 2.组件的宽高    3.是否允许拖拽（菜单开启状态）   4.静态标志位，用于记录当前组件是否正在被拖拽
bool HandleComponentDrag(ImVec2& pos, ImVec2 size, bool is_active, bool& is_dragging_state) {
    if (!is_active) return false;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;

    //检测鼠标是否在矩形内
    bool hovered = (mousePos.x >= pos.x && mousePos.x <= pos.x + size.x &&
        mousePos.y >= pos.y && mousePos.y <= pos.y + size.y);

    //按下左键：如果鼠标悬停，标记为开始拖拽
    if (hovered && io.MouseClicked[0]) {
        is_dragging_state = true;
    }

    //松开左键：停止拖拽
    if (io.MouseReleased[0]) {
        is_dragging_state = false;
    }

    //执行拖拽：更新坐标
    if (is_dragging_state && io.MouseDown[0]) {
        pos.x += io.MouseDelta.x;
        pos.y += io.MouseDelta.y;

        // 绘制调试边框 (黄色)，方便用户知道自己在拖谁
        ImGui::GetBackgroundDrawList()->AddRect(
            pos,
            { pos.x + size.x, pos.y + size.y },
            ImColor(255, 255, 0, 200)
        );

        return true; //执行了拖拽
    }

    return false;
}
