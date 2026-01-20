#include <Windows.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <GL/GL.h>
#include <tchar.h>
#include <iostream>
#include "MinHook64/MinHook.h"
#include "imgui_gl/imgui.h"
#include "imgui_gl/imgui_impl_win32.h"
#include "imgui_gl/imgui_impl_opengl3.h"
#include "gui/gui.h"
#include "cheat/cheat.h"
#include "cheat/EntityCache.h"
#include "Manager/ConfigManager.h"
#include "cheat/func_prototype.h"
#include "cheat/Patch.h"


#pragma (lib,"MinHook/minhook.x64d.lib")

// 定义原始函数指针
typedef BOOL(__stdcall* wglSwapBuffers_t)(HDC);
wglSwapBuffers_t o_wglSwapBuffers = nullptr;

// 定义 SDL 函数类型
typedef int(*SDL_SetRelativeMouseMode_t)(int enabled);
typedef int(*SDL_ShowCursor_t)(int toggle);

SDL_SetRelativeMouseMode_t SetRelativeMouseMode = nullptr;
SDL_SetRelativeMouseMode_t pSDL_SetRelativeMouseMode = nullptr;
SDL_ShowCursor_t pSDL_ShowCursor = nullptr;

// 全局变量用于标记初始化状态
bool is_initialized = false;
HWND game_window = nullptr;

// 原始的窗口消息处理函数 (用于输入)
WNDPROC o_WndProc = nullptr;

// 声明 ImGui 的输入处理函数 (在 imgui_impl_win32.cpp 中定义)
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//初始化函数
void InitSDLFuncs() {
    HMODULE hSDL = GetModuleHandleA("SDL2.dll");
    if (hSDL) {
        pSDL_SetRelativeMouseMode = (SDL_SetRelativeMouseMode_t)GetProcAddress(hSDL, "SDL_SetRelativeMouseMode");
        pSDL_ShowCursor = (SDL_ShowCursor_t)GetProcAddress(hSDL, "SDL_ShowCursor");
    }
}
//鼠标光标隐藏、显示逻辑
void assist_Cursor() {
    static bool init = false;
    if (!init) {
        HMODULE hSDL = GetModuleHandleA("SDL2.dll");
        if (hSDL) {
            SetRelativeMouseMode = (SDL_SetRelativeMouseMode_t)GetProcAddress(hSDL, "SDL_SetRelativeMouseMode");
        }
        init = true;
    }
    if (gui::windowFlag::menu && gui::windowFlag::click_lock) {

        //菜单开启并且锁定开启，显示鼠标
        while (ShowCursor(true) < 0);
        SetRelativeMouseMode(0);//关闭相对模式！
    }
    else {
        // 菜单关闭或者锁定关闭， 隐藏鼠标
        while (ShowCursor(false) >= 0);
        SetRelativeMouseMode(1);//开启相对模式！
    }
}

//新窗口消息处理函数 (为了让 ImGui 能响应鼠标键盘)
LRESULT __stdcall hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // --- 1. 优先让 ImGui 处理输入 ---
    // 即使 ImGui 处理了，我们稍后也可能需要根据菜单状态决定是否传给游戏
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    //insert，锁定聚焦窗口
    if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) {
        if (gui::windowFlag::menu)//只有在开启菜单的时候才可以选择锁定
            gui::windowFlag::click_lock = !gui::windowFlag::click_lock;//切换锁定
        else 
            gui::windowFlag::click_lock = false;//菜单关闭，就取消锁定

        if (gui::windowFlag::menu) {
            //关闭相对模式，让鼠标不要再锁定在中心了
            pSDL_SetRelativeMouseMode(0);
        }
        else {
            // 菜单关闭：
            // 恢复 FPS 模式，锁定鼠标
            pSDL_SetRelativeMouseMode(1);
        }
    }

    //home，菜单显示
    if (uMsg == WM_KEYDOWN && wParam == VK_HOME) {
        gui::windowFlag::menu = !gui::windowFlag::menu;//切换菜单显示
    }

    assist_Cursor();//检查标志位，决定是否隐藏光标

    // 屏蔽输入事件，只有click_lock 和 menu == true的时候才锁定
    if (gui::windowFlag::menu && gui::windowFlag::click_lock) {
        switch (uMsg) {
            // 屏蔽鼠标消息
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
            // 屏蔽键盘消息 (除了 INSERT，因为我们需要它来关闭菜单)
            // 注意：如果你需要在菜单里输入文字(比如输入框)，则不能屏蔽 WM_CHAR 或 WM_KEYDOWN
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
            // 这里做一个特殊判断：如果是 INSERT 键，放行（否则你关不掉菜单了）
            if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) break;

            // 彻底拦截：返回 0 或 1，骗过系统说“这个消息我已经处理完了”
            // 游戏原本的 WndProc 就永远收不到这些消息了 -> 不会开枪，不会移动
            return 0;
        }
    }

    // --- 5. 正常传递给游戏 ---
    return CallWindowProc(o_WndProc, hWnd, uMsg, wParam, lParam);
}

// 我们的 Hook 函数：拦截每一帧的渲染
BOOL __stdcall hk_wglSwapBuffers(HDC hDc) {
    if (!is_initialized) {
        //通过 DC 反推窗口，获取窗口句柄 
        game_window = WindowFromDC(hDc);
        //让我们的窗口过程函数优先处理信息
        o_WndProc = (WNDPROC)SetWindowLongPtr(game_window, GWLP_WNDPROC, (LONG_PTR)hWndProc);


        //初始化 ImGui
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(game_window);
        ImGui_ImplOpenGL3_Init(); 

        //初始化
        cheat::Init();//初始化各种变量以及模块基址
        Config::Init();//初始化配置
		func::Init();//初始化函数地址
        Patch::Init();//初始化补丁
        Hooks::Init();//hook初始化
        gui::Init();//绘制初始化

        is_initialized = true;
    }

    //开始渲染
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    //更新窗口大小数据，以及其他需要更新的数据
    Config::update();
	//更新缓存的实体数据
    EntityManager.Update();
    //绘制画面总成
    gui::Draw();
	//作弊功能运行
    cheat::Run();
    //彩虹变化颜色
    color::RanbowChange();

    // 渲染结束
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 调用原始函数，让游戏正常显示画面
    return o_wglSwapBuffers(hDc);
}

// DLL 入口
DWORD WINAPI MainThread(LPVOID lpReserved) {
    // 初始化 MinHook
    if (MH_Initialize() != MH_OK) return 0;

    // 获取 opengl32.dll 中的 wglSwapBuffers 地址
    HMODULE hMod = GetModuleHandleA("opengl32.dll");
    void* pTarget = (void*)GetProcAddress(hMod, "wglSwapBuffers");

    InitSDLFuncs();
    // 创建 Hook
    if (MH_CreateHook(pTarget, &hk_wglSwapBuffers, (void**)&o_wglSwapBuffers) != MH_OK) {
        return 0;
    }
    // 启用 Hook
    MH_EnableHook(MH_ALL_HOOKS);

    return 1;
}
//控制台线程，调试时启动
DWORD _stdcall ConsoleThread(LPVOID lpReserved) {
    using namespace std;

    //分配控制台
    AllocConsole();

    //控制台标题
    SetConsoleTitleA("DLL Debug Console");

    FILE* f;
    //重定向标准输出
    freopen_s(&f, "CONOUT$", "w", stdout);
    //重定向标准输入
    freopen_s(&f, "CONIN$", "r", stdin);
    //重定向标准错误
    freopen_s(&f, "CONOUT$", "w", stderr);

    //清理残留
    std::cin.clear();

    printf("=== DLL已加载 ===\n");
    //printf("可用指令: /saveconfig, /loadconfig\n");
    printf("射线检测函数地址%p\n", func::IntersectRay);
    printf("人物移动函数地址%p\n", func::MovePlayer);
    printf("摄像机指针%p\n", ct_var::camera);
    string command;
    const string setconfig = "/setconfig";
    while (1) {
        cout << "> "; //为了美观添加
        
        getline(cin, command);
        if (command == "/saveconfig") {
            Config::SaveConfig();
            cout << "[+] 保存配置成功!" << endl;
        }
        else if (command == "/loadconfig") {
            Config::LoadConfig();
            cout << "[+] 读取配置成功!" << endl;
        }else if(command == "/help"){
            cout << "可用指令: /saveconfig, /loadconfig, /help, /setconfig, /lock" << endl;
        }
        else if (command.find(setconfig) == 0) {
            string path = ".\\";
            if (command.length() > setconfig.length()) {
				path += command.substr(setconfig.length() + 1); // 提取文件名
                Config::config_path = path;
                cout << "[+] 配置路径:" << path << endl;
            }
            else {
				cout << "[-] 请提供配置文件名" << endl;
            }
        }
        else if (command.find("/lock") == 0) {
			gui::windowFlag::click_lock = true;
        }
        else {
            cout << "[-] 未知指令" << endl;
        }
    }

    //清理工作
    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
    return 0;
}
BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hMod);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        //调试手段↓
        //CreateThread(nullptr, 0, ConsoleThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
