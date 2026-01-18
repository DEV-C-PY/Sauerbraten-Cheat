#include "../imgui_gl/imgui.h"
#include "../imgui_gl/imgui_internal.h"

//存放标志位
namespace gui {
	namespace windowFlag {
		inline bool menu = true;//菜单开关
		inline bool EditColor = false;//调色板开关
		inline bool click_lock = true;
	}
}
//存放函数
namespace gui {
	//加载主题
	void SetupStyle();
	//初始化字体
	void Init();
	//绘制总成
	void Draw();
	//旧菜单
	void Menu();
	//新菜单
	void Menu1();
	//画布绘制，主要是绘制透视和过滤圈
	void Canvas();
	//调色板窗口
	void EditColor();
	//输出调试信息
	void Debug();
}

//我们对ImGui函数的封装
namespace UI {
	//封装多一层主要是为了我们好进行标志位的更改
	//封装版本
	bool Checkbox(const char* label, bool* v);
	//封装版本
	bool RadioButton(const char* label, int* v, int v_button);
	//封装版本
	bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	//封装版本
	bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	//这是一个自定义的 Toggle 控件
	bool ToggleButton(const char* str_id, bool* v);
	// 辅助函数：绘制侧边栏 Tab 按钮
	// label: 按钮文字
	// index: 这个按钮对应的 tab 索引 (0, 1, 2...)
	// selected_index: 当前选中的索引的引用
	// size: 按钮大小
	void TabButton(const char* label, int index, int* selected_index, ImVec2 size);
	
	//网址文本
	void TextURL(const char* name, const char* url, bool sameLineBefore = false);
}

//ImGui扩展
namespace ImGui {
	static ImVector<ImRect> s_GroupPanelLabelStack;
	void BeginGroupPanel(const char* name, const ImVec2& size);
	void EndGroupPanel();
}