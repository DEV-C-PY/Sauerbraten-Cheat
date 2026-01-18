#pragma once
#include <vector>
#include "../vec3/Vector.h"
#include "../imgui_gl/imgui.h"

class Entity {
private:
	bool IsInit{};
	int index{};//该人物在游戏人物数组里的下标！
public:
	//游戏原本数据字段
	void* EntityPtr{};
	char* name{};
	int blood{};
	float Yaw{};//实体的朝向角度
	float Pitch{};//实体的俯仰角度
	Vector3 worldPos{};//实体的世界坐标
	//实体状态	0生	1死	5幽灵(观战模式)
	unsigned char EntityState{};
	unsigned char team{};
public:
	//自定义数据字段
	bool IsInScreen{};//是否在屏幕内
	bool IsVisible{};//是否物理可见（判断与实体之间是否存在障碍物）
	bool IsInCircle{};//是否在过滤圈内
	float distance{};//与本地玩家的距离
	float pointDistance{};//与准心的距离
	Vector3 ScreenHeadPos{};//实体头部在屏幕的坐标	z为0
	Vector3 ScreenFeetPos{};//实体脚部在屏幕的坐标	z为0

public:
	//初始化实体类
	//1.实体在数组的下标
	Entity(int index);
	//空构造函数
	Entity() = default;
	//是否初始化
	bool isInit();
	//实体是否存活
	bool IsAlive();
	//实体是否处于幽灵状态(观战模式)
	bool IsGhost();
	//返回实体在实体数组的下标	-1代表没有初始化
	int MyIndex();
};
//全局只存在一个的实例的类！
//负责更新实体数据、状态的类
class EntityCache {

public:
	//更新敌人实体缓存
	void Update();
	//更新本地玩家实体缓存
	void UpdateLocalPlayer();
	//更新最佳目标
	void UpdateBestTarget();
	//敌人！实体列表
	std::vector<Entity> EntityList;
	//最佳敌人！
	//战斗功能应该优先选择这个目标
	Entity bestTarget;
	//最近敌人的游戏下标
	int DistanceClosestTargetIndex = -1;
	//准心最近敌人的游戏下标
	int SightClosestTargetIndex = -1;
	//敌人数量
	int EnemyNum{};
	//可见敌人数量
	int VisibleEntityNum{};
	//本地人物
	Entity LocalPlayer;
	//按下标取得缓存中的敌人实体
	Entity& operator[](int i);
};
//EntityCache类的全局唯一实例
inline EntityCache EntityManager;