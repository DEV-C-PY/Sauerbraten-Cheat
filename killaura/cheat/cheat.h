#pragma once
#include <Windows.h>
#include <string>
#include "../Manager/HookManager.h"
#include "../MinHook64/MinHook.h"
#include "EntityCache.h"
#include "../vec3/Vector.h"
#include "func_prototype.h"

#define CHEAT inline const uintptr_t
#define ct_Flag cheat::Flag
#define ct_baseAddr cheat::baseAddr
#define ct_var cheat::var
#define ct_Player cheat::localPlayer
#define ct_offsets cheat::offsets
#define ct_Patch cheat::Patch

//存放标志位，模块基址
namespace cheat {
	namespace Flag {//表示状态的标志
		inline bool isInit = false;
	}
	namespace baseAddr {//模块基址
		inline HMODULE gameBase{};
	}
}
//存放本地玩家变量，常用变量
namespace cheat {
	namespace var {//一些常用的变量
		inline uintptr_t* players{};//人物指针数组
		inline float* worldpos_x{};
		inline float* worldpos_y{};
		inline float* worldpos_z{};
		inline DWORD* gamePlayerNum{};//局内人数
		inline float* mvpMatrix{};//mvp矩阵
		inline int localPlayerIndex{};//本地玩家在人物数组的下标，一般为0
		inline int* lastmillis{};//全局时间戳
		inline uintptr_t* LocalPlayer2{};//人物指针二，解引用获得指针
		inline uintptr_t* camera{};//摄像机指针
		inline bool* thirdPerson{};//第三人称状态
		inline uintptr_t* octree{};//八叉树指针
	}
	namespace localPlayer {//本地玩家变量
		inline char* name{};
		inline int* blood{};
		inline float* x{};//坐标
		inline float* y{};
		inline float* z{};
		inline float* Yaw{};
		inline float* Pitch{};
		inline float* Speed{};
		inline BYTE* team{};
		inline BYTE* AdvanceState{};//人物前进状态，0没动，1前进，-1后退
		inline BYTE* LeftState{};//人物左右行走状态，0没动，1左，-1右
		inline BYTE* state{};//玩家状态0生	1死	4飞行	5幽灵(观战模式)
		inline bool* fire_state{};//是否开火状态
		inline int* shoot_delay{};//开火延迟，int类型，单位毫秒？
		inline float* vectorX{};//向量X
		inline float* vectorY{};//向量Y
		inline float* vectorZ{};//向量Z
		inline float* vector_x{};//向量X
		inline float* vector_y{};//向量Y
		inline float* vector_z{};//向量Z
	}
}
//存放偏移
namespace cheat {
	namespace offsets {//常用偏移
		//相对于人物指针
		CHEAT name = 0x274;
		CHEAT blood = 0x178;
		CHEAT loc_x = 0x30;//位置x坐标
		CHEAT loc_y = 0x34;
		CHEAT loc_z = 0x38;
		CHEAT Yaw = 0x3c;
		CHEAT Pitch = 0x40;
		CHEAT Speed = 0x48;//人物移动速度
		CHEAT AdvanceState = 0x74;//人物前进状态，0没动，1前进，-1后退
		CHEAT LeftState = 0x75;//人物左右行走状态，0没动，1左，-1右
		CHEAT State = 0x77;//BYTE	0生	1死	5幽灵(观战模式)
		CHEAT Team = 0x378;//BYTE	103蓝	101红
		//人物移动向量：前进，跳跃
		CHEAT vectorX = 0xc;//向量X
		CHEAT vectorY = 0x10;//向量Y
		CHEAT vectorZ = 0x14;//向量Z
		//环境向量：推力，重力
		CHEAT vector_x = 0x18;//向量X
		CHEAT vector_y = 0x1C;//向量Y
		CHEAT vector_z = 0x20;//向量Z
		CHEAT bullet1 = 0x194;//显示子弹
		CHEAT bullet2 = 0x35c;//显示子弹
		CHEAT hitRange = 0x4c;//(可能是)碰撞体半径偏移，人物指针+0x4c
		CHEAT axis_z_sub = 0x50;//(可能是)人物身高，专用于射线检测的z轴减数
		CHEAT fire_state = 0x1fc;//开火状态偏移，bool类型
		CHEAT shoot_delay = 0x190;//开火延迟偏移，int类型，单位毫秒？

		//相对于模块
		CHEAT players = 0x00346C90;//人物数组偏移
		CHEAT worldpos_x = 0x32BF00 - 8;//子弹落点x坐标
		CHEAT worldpos_y = 0x32BF00 - 4;
		CHEAT worldpos_z = 0x32BF00;
		CHEAT gamePlayerNum = 0x346C9C;//局内人数
		CHEAT mvpMatrix = 0x32D040;//Mvp矩阵
		CHEAT lastmillis = 0x26CD4C + 0x1000;//全局时间戳
		CHEAT camera = 0x2A1560 + 0x1000;//摄像机指针偏移
		CHEAT thirdPerson = 0x32CFA8;//第三人称偏移
		//八叉树偏移，相对于模块。重要！如果内容为空调用BulletTrace函数会引发内存访问异常！
		CHEAT octree = 0x2AAFD0 + 0x1000;
		//人物指针二？
		CHEAT LocalPlayer2_1 = 0x3472E0;//第一层偏移
		CHEAT LocalPlayer2_2 = 0;//第二层偏移
		CHEAT P2_delay = 0x358;//人物指针2的武器延迟

		//劫持地址，游戏调用函数的地址call gamefunc
		//call子弹落点计算函数的偏移
		//旧子弹追踪功能专用
		CHEAT call_bulletTrace = 0x145FD2 + 0x1000;
	}
}
//存放函数
namespace cheat {
	//初始化各种变量，以及模块基址。初始化应该优先调用
	//因为很多功能需要用到模块基址
	void Init();
	//初始化人物指针2
	void InitLocalPlayer2();
	//当人数发生较大变化，人物数组会被重新分配地址！！！，需要更新地址
	void UpdateArray();
	//简单作弊功能运行
	void Run();
}


//存放函数
namespace Hooks {
	//初始化hook，启用全部函数hook
	void Init();
	//Hook函数
	void* HookShoot(void* localPlayerPtr, Vector3* worldpos);
	void* HookFireEvent(void* UnKnownPtr, void* LocalPlayerPtr2);
	bool HookCollide(void* player, void* p2, void* p3, bool v4, bool v5);
	bool HookMovePlayer(void* player, int precision);
	void HookModifyGravity(void* player, bool v2, int v3);
	void HookModifyVec(void* player, int v2, bool v3, bool v4, int v5);
	bool HookMove(void* player, Vector3* vec, void* p3);
}
//存放被hook后游戏的原始函数指针
namespace Hooks {
	inline tShoot o_Shoot{};//shoot函数原始指针
	inline tFireEvent o_FireEvent{};//FireEvent函数原始指针
	inline tCollide o_Collide{};//Colide函数原始指针
	inline tMovePlayer o_MovePlayer{};//MovePlayer函数原始指针
	inline tModifyGravity o_ModifyGravity;//ModifyGravity函数原始指针
	inline tModifyVec o_ModifyVec;//ModifyVec函数原始指针
	inline tMove o_Move{};//Move函数原始指针
}

//自瞄
//1.需要瞄准的位置的世界坐标	2.平滑等级(越大越慢)
void AimBot(Vector3& targetPos,int SmoothLevel = 0);
//自动开火	检测瞄准到敌人时自动开火
void AutoFire();

//辅助函数
float NormalizeAngle(float angle);
//检查当前视角的射线是否能看见实体！
bool CheckEntityVisibility(Entity& entity);
//计算指向坐标的射线落点，并且负责还原Pitch和Yaw	调用方记得还原worldpos_x,y,z！！！！
//此射线受障碍物阻挡
//1.世界坐标
void LandingPointCal(Vector3& worldpos);

//旧艺术
//功能模块父类
class ICheatFeature {
public:
	virtual ~ICheatFeature() = default;

	virtual const char* GetName() const = 0;  // 功能名称
	virtual void Enable() = 0;                 // 开启
	virtual void Disable() = 0;                // 关闭
	virtual void Toggle() {                    // 切换
		m_enabled ? Disable() : Enable();
	}
	virtual bool IsEnabled() const { return m_enabled; }

protected:
	bool m_enabled = false;
};

//无参数，无返回值
typedef void(__cdecl* v2)(void);
class bulletTrace : public ICheatFeature {
private:
	HookInfo m_hook;
	void* blank, * blank1;
	static void __cdecl HookFunc();

public:
	const char* GetName() const override { return "子弹追踪"; }
	//实现流程:
	// call gamefunc -> 
	// {jmp 空白		跳到空白区放置我们自己的汇编逻辑
	// call gamefunc	调用原本位置的游戏函数，执行原本逻辑
	// jmp 空白1		这片空间没位置了，继续跳转新的空白区
	// call HookFunc	重点！执行我们的作弊逻辑
	// jmp 正常流程		跳转回去执行正常的游戏流程
	// }
	void Enable() override;
	//将jmp 空白 改回 call gamefunc即可
	void Disable() override;
};
