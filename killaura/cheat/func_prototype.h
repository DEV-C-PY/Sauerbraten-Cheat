#pragma once
#include "../vec3/Vector.h"

#define FUNC inline const uintptr_t
#define fc_offsets func::offsets
//文件用于声明游戏的函数原型！

//射线与碰撞体检测函数原型
//1.射线起点	2.射线终点	3.敌人脚部坐标	4.敌人头部坐标	5.敌人碰撞体半径	6.输出结果（无用）
typedef bool(__fastcall* tIntersectRay)(
	Vector3* vecRayStart,      // rcx: 射线起点
	Vector3* vecRayEnd,        // rdx: 射线终点
	Vector3* vecEnemyBottom,   // r8: 敌人脚部坐标
	Vector3* vecEnemyTop,      // r9: 敌人头部坐标
	float    fHitboxRadius,    // stack[0x20]: 可能是敌人碰撞体半径 (人物指针 + 0x4c)
    float*	pOutHitTime		   // Stack[0x28]: 接收输出结果 (无用)
    );
//子弹落点计算函数原型
typedef void*(__fastcall* tBulletTrace)(void);
//射击函数原型
//1.本地玩家指针	2.子弹落点坐标指针
typedef void* (__fastcall* tShoot)(
	void* localPlayer,		  //1.本地玩家指针
	Vector3* worldpos		  //2.子弹落点坐标指针
	);
//开火事件函数，处理延迟和子弹2扣除的函数
//1.未知类型指针（在人物指针2下的某变量）	2.人物指针二
typedef void* (__fastcall* tFireEvent)(
	void* UnKnownPtr,//1.未知类型，未知作用的指针（我们有稳定指针链，但是作用未知）
	void* LocalPlayer2//2.人物指针二
	);
//人物移动函数
//1.人物指针（敌人也会进入此函数）	2.碰撞检测精度？	注意处理自己和敌人的逻辑
typedef bool(__fastcall* tMovePlayer)(
	void* localPlayer,	//玩家指针
	int precision		//精度
	);
//碰撞检测函数
//1.人物指针	2 - 5暂未研究
typedef bool(__fastcall* tCollide) (
	void* player,
	void* ptr1,
	void* ptr2,
	bool v4,
	bool v5
	);
//重力检查函数	函数产生的效果->人物指针+0x20的浮点数-1（下坠时），0x20的浮点数赋值为0（在陆地行走时)
//无返回值,void
//1.人物指针	2.未知，bool类型	3.未知，int类型 通常为5
typedef void(__fastcall* tModifyGravity) (
	void* player,
	bool v2,
	int v3
	);
//人物移动与合法性检测函数	
//返回bool类型？未知返回值的意思
//1.人物指针	2.向量结构体！（3个float值）	3.未知指针
typedef bool(__fastcall* tMove) (
	void* player,
	Vector3* vec,
	void* p3
	);
//向量产生函数  影响速度
//无返回值
//1.人物指针	2.int未知	3.bool未知	4.bool未知	5.int未知
typedef void(__fastcall* tModifyVec)(
	void* player,
	int v2,
	bool v3,
	bool v4,
	int v5
	);
//存放函数
namespace func {
	//初始化函数指针
	void Init();
}
//存放游戏函数指针
namespace func{
	//射线相交碰撞体检测函数
	//返回bool值，表示是否相交
	//1.射线起点	2.射线终点	3.敌人脚部坐标	4.敌人头部坐标	5.敌人碰撞体半径	6.输出结果（无用）
	inline tIntersectRay IntersectRay = nullptr;
	//子弹落点计算函数
	//返回未知类型的指针，返回值一般无用
	//无参数
	inline tBulletTrace BulletTrace = nullptr;
	//射击函数
	//返回未知类型的指针，返回值一般无用。有可能是void
	//1.本地玩家指针	2.子弹落点坐标指针
	inline tShoot Shoot = nullptr;
	//开火事件函数，处理延迟和子弹2扣除		子弹2指的是人物指针2的子弹数量
	//返回未知类型的指针，返回值对我们无用
	//1.未知类型指针（在人物指针2下的变量）	2.人物指针二
	inline tFireEvent FireEvent = nullptr;
	//人物移动函数
	//返回bool类型，代表移动成功与否？
	//1.人物指针（敌人也会进入此函数）	2.碰撞检测精度？	注意处理自己和敌人的逻辑
	inline tMovePlayer MovePlayer = nullptr;
	//碰撞检测函数
	//返回bool类型，表示是否碰到墙了
	//1.人物指针	2 - 5暂未研究
	inline tCollide Collide = nullptr;
	//重力检查函数	
	//下坠时：人物指针+0x20位置的浮点数（下向量）持续递减		平地行走时：浮点数 = 0
	//无返回值,void
	//1.人物指针	2.未知，bool类型	3.未知，int类型 通常为5
	inline tModifyGravity ModifyGravity = nullptr;
	//向量产生函数  影响速度
	//无返回值
	//1.人物指针	2.int未知	3.bool未知	4.bool未知	5.int未知
	inline tModifyVec ModifyVec = nullptr;
	//人物移动与合法性检测函数	
	//返回bool类型？未知返回值的意思
	//1.人物指针	2.向量结构体！（3个float值）	3.未知指针
	inline tMove Move = nullptr;

	
}
//存放函数的偏移
namespace func {
	//存储函数相对于模块的偏移
	namespace offsets {
		//子弹落点计算函数的偏移	
		FUNC func_bulletTrace = 0x146350 + 0x1000;
		//射线与碰撞体检测函数的偏移
		FUNC func_IntersectRay = 0x1B0A00 + 0x1000;
		//射击函数的偏移
		FUNC func_shoot = 0x1DA4C0 + 0x1000;
		//开火事件函数的偏移
		FUNC func_FireEvent = 0x1EB330 + 0x1000;
		//人物移动函数的偏移
		FUNC func_MovePlayer = 0x10DF90 + 0x1000;
		//碰撞检测函数的偏移
		FUNC func_Collide = 0x111310 + 0x1000;
		//重力检查函数的偏移
		FUNC func_ModifyGravity = 0x10E490 + 0x1000;
		//Move函数的偏移
		FUNC func_Move = 0x10F930 + 0x1000;
		//ModifyVec函数的偏移
		FUNC func_ModifyVec = 0x10E690 + 0x1000;
	}
}