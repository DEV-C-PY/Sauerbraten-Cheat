#include "EntityCache.h"
#include "cheat.h"
#include "../imgui_gl/imgui.h"
#include "Esp.h"
#include "../Manager/ConfigManager.h"
#include "func_prototype.h"

bool Entity::isInit() {
	return this->IsInit;
}
bool Entity::IsAlive() {
	if (this->IsInit) return this->EntityState == 0x0;
	return false;
}
bool Entity::IsGhost() {
	if (this->IsInit) return this->EntityState == 0x5;
	return false;
}
int Entity::MyIndex() {
	if (this->IsInit) return this->index;
	return -1;
}
Entity::Entity(int index) {
	this->IsInit = false;
	if (index < 0 || index >= *ct_var::gamePlayerNum || ct_var::players[index] == 0) {
		std::string error = "Entity初始化失败，index越界：" + std::to_string(index);
		MessageBoxA(0, "Entity初始化失败", error.c_str(), MB_OK);
		return;
	}
	else {
		this->index = index;//记录实体在游戏里的实体数组的下标！
		this->EntityPtr = (void*)ct_var::players[index];
		this->name = (char*)(ct_var::players[index] + ct_offsets::name);
		this->blood = *(int*)(ct_var::players[index] + ct_offsets::blood);
		this->Yaw = *(float*)(ct_var::players[index] + ct_offsets::Yaw);
		this->Pitch = *(float*)(ct_var::players[index] + ct_offsets::Pitch);
		this->worldPos = Vector3(
			*(float*)(ct_var::players[index] + ct_offsets::loc_x),
			*(float*)(ct_var::players[index] + ct_offsets::loc_y),
			*(float*)(ct_var::players[index] + ct_offsets::loc_z));
		this->EntityState = *(BYTE*)(ct_var::players[index] + ct_offsets::State);
		this->team = *(BYTE*)(ct_var::players[index] + ct_offsets::Team);
		this->IsInit = true;
	}
}


void EntityCache::Update() {
	this->EntityList.clear();
	cheat::UpdateArray();//更新人物数组地址
	//屏幕尺寸，Config更新
	const float screenW = Config::window_size.x;
	const float screenH = Config::window_size.y;
	//MVP矩阵指针
	static float* mvpMatrixPtr = ct_var::mvpMatrix;
	//更新本地玩家
	UpdateLocalPlayer();
	//重置可见实体数量
	this->VisibleEntityNum = 0;
	//更新敌人实体缓存
	for (int i = 1; i < *ct_var::gamePlayerNum; ++i) {//从下标1开始，跳过本地玩家
		Entity cache(i);//构建敌人实体，初始化大部分字段

		if (!cache.isInit()) continue;//实体没有初始化成功，跳过

		//计算头部和脚部屏幕坐标
		Vector3 enemyWorldPos = cache.worldPos;
		//判断敌人是否在屏幕内，并且计算屏幕坐标	只要有一个点在屏幕内就算可见
		if (WorldToScreen(enemyWorldPos, cache.ScreenHeadPos, mvpMatrixPtr, screenW, screenH)) cache.IsInScreen = true;
		enemyWorldPos.z -= 15.0f; //人物大概高15，计算脚部位置
		if (WorldToScreen(enemyWorldPos, cache.ScreenFeetPos, mvpMatrixPtr, screenW, screenH)) cache.IsInScreen = true;
		//计算距离
		Vector3& localWorldPos = this->LocalPlayer.worldPos;
		cache.distance = (cache.worldPos - localWorldPos).Length();
		cache.distance *= 0.1f;//我们只需要知道一个可靠描述距离的数值，所以*0.1，数值过大观感不好
		//计算准心距离
		Vector3 screenCenter = Vector3(screenW / 2.0f, screenH / 2.0f, 0.0f);
		Vector3 subVec = (cache.ScreenHeadPos - screenCenter);
		cache.pointDistance = sqrt((subVec.x * subVec.x + subVec.y * subVec.y));//勾股定理计算2D距离
		//判断是否处于过滤圈内，如果不存在于屏幕里，那肯定不在圈内，不用else
		if (cache.IsInScreen)cache.IsInCircle = (cache.pointDistance < Cg_combat::circle_radius);
		//可视化判断（掩体判断\与实体之间是否存在障碍物）
		//若未开启过滤不存于屏幕实体选项，则即使不存在屏幕里的敌人也要进行判断
		if (cache.IsInScreen || !Cg_general::filter_inscreen) {
			//备份worldpos_x,y,z，最后需要还原
			float tempWorldPos_x = *ct_var::worldpos_x;
			float tempWorldPos_y = *ct_var::worldpos_y;
			float tempWorldPos_z = *ct_var::worldpos_z;
			//准备用于射线相交检测的参数
			LandingPointCal(cache.worldPos);//计算射线落点
			cache.IsVisible = CheckEntityVisibility(cache);
			//还原worldpos_x,y,z
			*ct_var::worldpos_x = tempWorldPos_x;
			*ct_var::worldpos_y = tempWorldPos_y;
			*ct_var::worldpos_z = tempWorldPos_z;
		}
		//记录可见人数
		if (cache.IsVisible) {
			bool add_ = true;
			if (Cg_general::filter_teammate && cache.team == this->LocalPlayer.team) add_ = false;//过滤队友
			if (cache.IsGhost() || !cache.IsAlive()) add_ = false;//过滤幽灵，过滤死人
			if(add_) this->VisibleEntityNum += 1;
		}
		//存入容器
		this->EntityList.push_back(cache);
	}
	//更新敌人数量
	this->EnemyNum = this->EntityList.size();
	//开始选取最佳目标
	UpdateBestTarget();
}
void EntityCache::UpdateLocalPlayer() {
	this->LocalPlayer = Entity(ct_var::localPlayerIndex);
}
void EntityCache::UpdateBestTarget() {
	int min_distance = INT_MAX;//距离最近
	int min_sight = INT_MAX;//准心距离
	this->DistanceClosestTargetIndex = -1;
	this->SightClosestTargetIndex = -1;
	this->bestTarget = Entity();//执行空构造，假设没有最佳目标
	for (int i = 0; i < this->EnemyNum; ++i) {
		Entity& enemy = this->EntityList[i];

		if (Cg_general::filter_teammate && enemy.team == this->LocalPlayer.team) continue;//过滤队友
		if (Cg_general::filter_ghost && enemy.IsGhost()) continue;//过滤幽灵
		if (Cg_general::filter_dead && !enemy.IsAlive()) continue;//过滤死人
		if (Cg_general::filter_invisible && !enemy.IsVisible) continue;//过滤不可见敌人（障碍物遮挡）
		if (Cg_general::filter_inscreen && !enemy.IsInScreen) continue;//过滤不存在于屏幕中的敌人
		if (Cg_general::filter_circle && !enemy.IsInCircle) continue;//过滤圈外敌人

		if (min_distance > enemy.distance) {
			min_distance = enemy.distance;
			this->DistanceClosestTargetIndex = enemy.MyIndex();//不在循环内进行构造！我们记录下标，循环结束后再构造最佳目标实体
		}
		if (min_sight > enemy.pointDistance) {
			min_sight = enemy.pointDistance;
			this->SightClosestTargetIndex = enemy.MyIndex();//不在循环内进行构造！我们记录下标，循环结束后再构造最佳目标实体
		}

	}

	//构建最佳目标
	switch (Cg_combat::CTMode) {
		case choiceMode::distance:
			if (this->DistanceClosestTargetIndex != -1) {
				this->bestTarget = this->EntityList[DistanceClosestTargetIndex - 1];//数组下标就等于游戏下标-1
			}
			break;
		case choiceMode::sight:
			if (this->SightClosestTargetIndex != -1) {
				this->bestTarget = this->EntityList[SightClosestTargetIndex - 1];//数组下标就等于游戏下标-1
			}
			break;
		default:
			break;
	}

}
Entity& EntityCache::operator[](int i) {
	return this->EntityList[i];
}