#include <cmath>
#include <algorithm>
#include "cheat.h"
#include "EntityCache.h"
#include "func_prototype.h"
#include "../Manager/ConfigManager.h"
#include "Patch.h"

//存放函数
namespace cheat {
	//初始化
	void Init() {
		try {
			//baseAddr
			ct_baseAddr::gameBase = GetModuleHandleA(NULL);

			//var
			ct_var::players = (uintptr_t*)*(uintptr_t*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::players);//"sauerbraten.exe"+00346C90
			ct_var::worldpos_x = (float*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::worldpos_x);
			ct_var::worldpos_y = (float*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::worldpos_y);
			ct_var::worldpos_z = (float*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::worldpos_z);
			ct_var::gamePlayerNum = (DWORD*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::gamePlayerNum);
			ct_var::mvpMatrix = (float*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::mvpMatrix);
			ct_var::localPlayerIndex = 0;
			ct_var::lastmillis = (int*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::lastmillis);
			ct_var::camera = (uintptr_t*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::camera);
			ct_var::thirdPerson = (bool*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::thirdPerson);
			InitLocalPlayer2();
			ct_var::octree = (uintptr_t*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::octree);

			//Player
			ct_Player::name = (char*)(ct_var::players[0] + ct_offsets::name);
			ct_Player::blood = (int*)(ct_var::players[0] + ct_offsets::blood);
			ct_Player::x = (float*)(ct_var::players[0] + ct_offsets::loc_x);
			ct_Player::y = (float*)(ct_var::players[0] + ct_offsets::loc_y);
			ct_Player::z = (float*)(ct_var::players[0] + ct_offsets::loc_z);
			ct_Player::Yaw = (float*)(ct_var::players[0] + ct_offsets::Yaw);
			ct_Player::Pitch = (float*)(ct_var::players[0] + ct_offsets::Pitch);
			ct_Player::Speed = (float*)(ct_var::players[0] + ct_offsets::Speed);
			ct_Player::team = (BYTE*)(ct_var::players[0] + ct_offsets::Team);
			ct_Player::AdvanceState = (BYTE*)(ct_var::players[0] + ct_offsets::AdvanceState);
			ct_Player::LeftState = (BYTE*)(ct_var::players[0] + ct_offsets::LeftState);
			ct_Player::state = (BYTE*)(ct_var::players[0] + ct_offsets::State);
			ct_Player::fire_state = (bool*)(ct_var::players[0] + ct_offsets::fire_state);
			ct_Player::shoot_delay = (int*)(ct_var::players[0] + ct_offsets::shoot_delay);
			ct_Player::vectorX = (float*)(ct_var::players[0] + ct_offsets::vectorX);
			ct_Player::vectorY = (float*)(ct_var::players[0] + ct_offsets::vectorY);
			ct_Player::vectorZ = (float*)(ct_var::players[0] + ct_offsets::vectorZ);
			ct_Player::vector_x = (float*)(ct_var::players[0] + ct_offsets::vector_x);
			ct_Player::vector_y = (float*)(ct_var::players[0] + ct_offsets::vector_y);
			ct_Player::vector_z = (float*)(ct_var::players[0] + ct_offsets::vector_z);
			//*ct_Player::Speed = 100.0f * Cg_Move::Speed;//初始化速度

			//Flag
			cheat::Flag::isInit = true;
		}
		catch (std::exception e) {
			MessageBoxA(0, "初始化失败", e.what(), 0);
			cheat::Flag::isInit = false;
		}
	}
	//初始化人物指针2
	void InitLocalPlayer2() {
		ct_var::LocalPlayer2 = (uintptr_t*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::LocalPlayer2_1);
		if (ct_var::LocalPlayer2 == nullptr) return;//这个指针在开始游戏才存在，避免越界访问
		ct_var::LocalPlayer2 = (uintptr_t*)(*ct_var::LocalPlayer2 + ct_offsets::LocalPlayer2_2);
	}
	//当人数发生较大变化，人物数组会被重新分配地址！！！，需要更新地址
	void UpdateArray() {
		ct_var::players = (uintptr_t*)*(uintptr_t*)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::players);//"sauerbraten.exe"+00346C90
	}
	//作弊功能执行
	void Run() {
		//自瞄处理
		if (Cg_AimBot::Enable) {
			if (EntityManager.bestTarget.isInit() && GetAsyncKeyState(Cg_AimBot::key)) {
				if (Cg_AimBot::mode == AimMode::smooth)
					AimBot(EntityManager.bestTarget.worldPos, Cg_AimBot::smooth);
				else if (Cg_AimBot::mode == AimMode::lock)
					AimBot(EntityManager.bestTarget.worldPos);
			}
		}
		//自动开火处理
		if (Cg_Func::E_AutoFire) AutoFire();
		//瞬移
		if (GetAsyncKeyState(VK_F2)) {
			if (EntityManager.bestTarget.isInit()) {
				Entity& bestTarget = EntityManager.bestTarget;
				*ct_Player::x = bestTarget.worldPos.x;
				*ct_Player::y = bestTarget.worldPos.y;
				*ct_Player::z = bestTarget.worldPos.z + 50.0f;
			}
		}
	}
}

//存放Hook函数实现
namespace Hooks {
	//启用全部hook
	void Init() {
		MH_CreateHook(func::Shoot, &Hooks::HookShoot, (void**)&Hooks::o_Shoot);
		MH_EnableHook(func::Shoot);
		MH_CreateHook(func::FireEvent, &Hooks::HookFireEvent, (void**)&Hooks::o_FireEvent);
		MH_EnableHook(func::FireEvent);
		MH_CreateHook(func::Collide, &Hooks::HookCollide, (void**)&Hooks::o_Collide);
		MH_EnableHook(func::Collide);
		MH_CreateHook(func::MovePlayer, &Hooks::HookMovePlayer, (void**)&Hooks::o_MovePlayer);
		MH_EnableHook(func::MovePlayer);
		MH_CreateHook(func::ModifyGravity, &Hooks::HookModifyGravity, (void**)&Hooks::o_ModifyGravity);
		MH_EnableHook(func::ModifyGravity);
		MH_CreateHook(func::ModifyVec, &Hooks::HookModifyVec, (void**)&Hooks::o_ModifyVec);
		MH_EnableHook(func::ModifyVec);
		MH_CreateHook(func::Move, &Hooks::HookMove, (void**)&Hooks::o_Move);
		MH_EnableHook(func::Move);
	}
	//无后坐力，射击延迟，禁止机器人开枪，子弹追踪
	void* HookShoot(void* localPlayerPtr, Vector3* worldpos) {
		//是否是本地玩家
		bool IsLocal = localPlayerPtr == EntityManager.LocalPlayer.EntityPtr;

		//非本地人物处理↓
		if (!IsLocal) {
			//机器人禁止开枪功能
			if (Cg_Func::H_BotFire) {
				bool* fire_state = (bool*)((uintptr_t)(localPlayerPtr)+ct_offsets::fire_state);
				*fire_state = false;
				return o_Shoot(localPlayerPtr, worldpos);
			}
			return o_Shoot(localPlayerPtr, worldpos);//非本地玩家并且没开启机器人禁止开枪直接走正常流程
		}

		//本地人物处理↓
		//无后坐力功能，需要分段处理
		float tempVX{};
		float tempVY{};
		float tempVZ{};
		if (Cg_Func::H_NoRecoil) {//记录原始向量
			tempVX = *ct_Player::vectorX;
			tempVY = *ct_Player::vectorY;
			tempVZ = *ct_Player::vectorZ;
		}

		//子弹追踪
		if (Cg_Func::H_BulletTrace) {
			Entity& best = EntityManager.bestTarget;
			if (best.isInit()) *worldpos = best.worldPos;//直接修改落点为敌人的坐标
		}

		//射击延迟功能
		if (Cg_Func::H_ShootDelay) {
			static bool largeZero = true;
			if (*ct_Player::shoot_delay > Cg_combat::shoot_delay)
				*ct_Player::shoot_delay = Cg_combat::shoot_delay;//设置自定义开火延迟

			if (*ct_Player::shoot_delay == 0) largeZero = true;

			void* ret = o_Shoot(localPlayerPtr, worldpos);//调用游戏原函数

			if (largeZero && *ct_Player::shoot_delay < Cg_combat::shoot_delay) {
				*ct_Player::shoot_delay = Cg_combat::shoot_delay;//设置自定义开火延迟
				largeZero = false;
			}
			//无后坐力处理
			if (Cg_Func::H_NoRecoil) {
				*ct_Player::vectorX = tempVX;//复原向量
				*ct_Player::vectorY = tempVY;
				*ct_Player::vectorZ = tempVZ;
			}
			return ret;
		}

		void* ret1 = o_Shoot(localPlayerPtr, worldpos);

		//无后坐力处理
		if (Cg_Func::H_NoRecoil && IsLocal) {//复原向量
			*ct_Player::vectorX = tempVX;
			*ct_Player::vectorY = tempVY;
			*ct_Player::vectorZ = tempVZ;
		}

		return ret1;
	}
	//协同处理射击延迟，不然射出的都是假子弹
	void* HookFireEvent(void* UnKnownPtr, void* LocalPlayerPtr2) {
		//此函数应该是本地人物专用，不用判断是否是本地人物

		//射击延迟功能↓
		if (Cg_Func::H_ShootDelay) {
			//这个指针是FireEvent函数内部重要的延迟判断变量，不修改则会射出假子弹，没有实际伤害！
			static int* delay = nullptr;
			if (ct_var::LocalPlayer2) {
				if (!delay) delay = (int*)(*ct_var::LocalPlayer2 + ct_offsets::P2_delay);
			}
			else {
				cheat::InitLocalPlayer2();
				delay = nullptr;
			}

			if (delay)	*delay = Cg_combat::shoot_delay;//自定义为我们自己设置的延迟
			void* ret = o_FireEvent(UnKnownPtr, LocalPlayerPtr2);

			if (delay) {
				if (*delay == Cg_combat::shoot_delay) {//射击延迟没被函数正确复位，证明这个指针已经被弃用了，需要更新
					cheat::InitLocalPlayer2();
					delay = nullptr;
				}
			}
			return ret;
		}

		return o_FireEvent(UnKnownPtr, LocalPlayerPtr2);//未开启功能就走正常流程
	}
	//全向穿墙
	//注：水平穿墙和纵向穿墙由补丁完成，不需要hook函数
	bool HookCollide(void* player, void* p2, void* p3, bool v4, bool v5) {
		//是否是本地玩家
		bool IsLocal = player == EntityManager.LocalPlayer.EntityPtr;

		//非本地人物处理↓
		if (!IsLocal) {
			return o_Collide(player, p2, p3, v4, v5);//非本地人物走正常流程
		}

		//本地人物处理↓
		if (Cg_Func::H_Collide && Cg_Move::IsALL_through()) return false;//开启全向穿墙，直接让碰撞检测函数返回false
		return o_Collide(player, p2, p3, v4, v5);//未开启全向穿墙就走正常流程
	}
	//飞行
	bool HookMovePlayer(void* player, int precision) {
		//是否是本地玩家
		bool IsLocal = player == EntityManager.LocalPlayer.EntityPtr;

		//非本地人物处理↓
		if (!IsLocal) {

			return o_MovePlayer(player, precision);//非本地人物走正常流程
		}

		//本地人物处理↓
		//飞行功能处理
		if (Cg_Func::H_Fly) {
			BYTE temp = *ct_Player::state;
			*ct_Player::state = 4;//暂时切换飞行状态
			bool ret = o_MovePlayer(player, precision);
			*ct_Player::state = temp;//复原
			return ret;
		}

		return o_MovePlayer(player, precision);//没开功能就走正常流程
	}
	//重力修改
	void HookModifyGravity(void* player, bool v2, int v3) {
		//是否是本地玩家
		bool IsLocal = player == EntityManager.LocalPlayer.EntityPtr;

		//非本地人物处理↓
		if (!IsLocal) {
			return o_ModifyGravity(player, v2, v3);//非本地人物走正常流程
		}

		//本地人物处理↓
		//自定义重力值功能处理↓
		if (Cg_Func::H_Gravity) {
			o_ModifyGravity(player, v2, v3);
			*ct_Player::vector_z = Cg_Move::Gravity;//设定为我们自己的重力值
			return;//返回
		}

		return o_ModifyGravity(player, v2, v3);//没开功能就走正常流程
	}
	//机器人禁止移动，灵活控制，无后摇移动，自动跳跃，围绕目标旋转
	void HookModifyVec(void* player, int v2, bool v3, bool v4, int v5) {
		//是否是本地玩家
		bool IsLocal = player == EntityManager.LocalPlayer.EntityPtr;

		//非本地人物处理↓
		if (!IsLocal) {
			//机器人禁止移动功能处理↓
			if (Cg_Func::H_BotMove) {
				Vector3* botVel = (Vector3*)((uintptr_t)player + ct_offsets::vectorX);
				botVel->x = 0; botVel->y = 0; botVel->z = 0;
				return;
			}
			return o_ModifyVec(player, v2, v3, v4, v5);//非本地人物走正常流程
		}

		//本地人物处理↓
		//围绕目标旋转
		bool IsRotate = false;//代表是否执行旋转
		if (Cg_Func::H_Rotate) {
			static bool enemy{};
			static uintptr_t enemyptr;

			//只要有最佳目标就更新目标的点
			if (EntityManager.bestTarget.isInit()) {
				enemyptr = (uintptr_t)EntityManager.bestTarget.EntityPtr;
				Cg_Recreation::point = (Vector3*)(enemyptr + ct_offsets::loc_x);
			}
			else Cg_Recreation::point = nullptr;
			static float tempSpeed = -1;//加速跟随，加速原本值
			static bool tempSpeedState = false;//原本加速状态

			//高度微调相关变量
			static bool Zset = false;//是否进行高度微调
			static bool tempJumpState = Cg_Func::H_AutoJump;//自动跳跃功能原值
			Vector3& LocalPos = EntityManager.LocalPlayer.worldPos;//获得本地人物的坐标

			//调整高度
			if (Cg_Recreation::point &&
				(LocalPos.z + 5.0f < Cg_Recreation::point->z ||
					LocalPos.z > Cg_Recreation::point->z + 40.0f)) {
				float distance = (*Cg_Recreation::point - LocalPos).Length2D() * 0.1f;//计算与目标的距离
				//只有在加速范围以内才进行调整
				if (distance < Cg_Recreation::setDistance + Cg_Recreation::setDeviation2) {
					if (LocalPos.z + 5.0f < Cg_Recreation::point->z) {
						*ct_Player::vectorZ += 5.0f;//我们高度太低了，修正一下z坐标
						*ct_Player::vector_z = 0;
					}
					else if (LocalPos.z > Cg_Recreation::point->z + 40.0f) {
						*ct_Player::vectorZ -= 5.0f;//我们高度太高了，修正一下z坐标
					}
					if (!Zset) tempJumpState = Cg_Func::H_AutoJump;
					Cg_Func::H_AutoJump = false;
					Zset = true;
				}
			}
			else Zset = false;
			//如果未进行高度微调，就复原功能
			//因为微调时会强制关闭自动跳跃功能
			if (!Zset) Cg_Func::H_AutoJump = tempJumpState;

			//移动才旋转
			if (Cg_Recreation::point && (*ct_Player::AdvanceState != 0 || *ct_Player::LeftState != 0)) {
				float distance = (*Cg_Recreation::point - LocalPos).Length2D() * 0.1f;//计算与目标的距离
				//如果人物移动时在 跟随距离+-偏差范围内就执行旋转
				if (distance < Cg_Recreation::setDistance + Cg_Recreation::setDeviation1 &&
					distance > Cg_Recreation::setDistance - Cg_Recreation::setDeviation1) {
					//如果进入旋转分支，并且Speed被赋值了，代表刚才进入过加速分支
					//需要复原
					if (tempSpeed != -1) {
						Cg_Move::Speed = tempSpeed;
						Cg_Func::H_Speed = tempSpeedState;
						tempSpeed = -1;
					}
					IsRotate = true;//在圈内执行旋转了，那就把标志位设置为true
					//保存变量
					float tempYaw = *ct_Player::Yaw;
					float tempPitch = *ct_Player::Pitch;
					BYTE tempAdvanceState = *ct_Player::AdvanceState;
					BYTE tempLeftState = *ct_Player::LeftState;
					o_ModifyVec(player, v2, v3, v4, v5);//这一步是为了让我们在旋转模式也能正常跳跃
					float temp = *ct_Player::vectorZ;//记录原值
					//自瞄到目标的角度
					AimBot(*Cg_Recreation::point);
					//左旋与右旋判断。+-90°是为了我们与目标连线后跟我们的视角形成一个直角
					//本质上就是一直对着目标的直角方向运行（画圆）
					if (*ct_Player::LeftState == -1)*ct_Player::Yaw += 90.0f;//往右走就右转
					else *ct_Player::Yaw -= 90.0f;//否则默认左转，或者本来就是左转
					//修正仰俯角
					*ct_Player::Pitch = 0.0f;
					//微调，当准备进圈或者出圈时，我们修正角度
					float adjustAngle = 45.0f;
					if (distance < Cg_Recreation::setDistance - 0.3f) {
						//执行出圈
						if (*ct_Player::LeftState == -1)*ct_Player::Yaw += adjustAngle;
						else *ct_Player::Yaw -= adjustAngle;
					}
					if (distance > Cg_Recreation::setDistance + 0.3f) {
						//执行进圈
						if (*ct_Player::LeftState == -1)*ct_Player::Yaw -= adjustAngle;
						else *ct_Player::Yaw += adjustAngle;
					}
					//准备变量
					//清零向量，让函数重新计算
					*ct_Player::vectorX = 0;
					*ct_Player::vectorY = 0;
					*ct_Player::LeftState = 0;//左右移动会影响速度
					*ct_Player::AdvanceState = 1;//强制前进
					for (int i = 0; i < 100; ++i) o_ModifyVec(player, v2, v3, v4, v5);//重新计算向量
					//还原变量
					*ct_Player::LeftState = tempLeftState;
					*ct_Player::AdvanceState = tempAdvanceState;
					*ct_Player::vectorZ = temp;
					*ct_Player::Pitch = tempPitch;
					*ct_Player::Yaw = tempYaw;

				}
				//如果人物在 跟随距离+偏差范围 至 跟随距离+加速范围 区间就直接直线加速跟随目标
				else if (distance > Cg_Recreation::setDistance + Cg_Recreation::setDeviation1 &&
					distance < Cg_Recreation::setDistance + Cg_Recreation::setDeviation2) {
					//如果进入加速分支，并且Speed没被赋值，那么就记录一下原值。
					//后面需要复原，我们只是暂时加速跟随到目标旁边
					if (tempSpeed == -1) {
						tempSpeed = Cg_Move::Speed;
						tempSpeedState = Cg_Func::H_Speed;
					}
					Cg_Func::H_Speed = true;//强制开启加速跟随至目标旁边
					Cg_Move::Speed += 0.1f;//渐进提升跟随速度
					IsRotate = true;//在圈内执行旋转了，那就把标志位设置为true
					//保存变量
					float tempYaw = *ct_Player::Yaw;
					float tempPitch = *ct_Player::Pitch;
					BYTE tempAdvanceState = *ct_Player::AdvanceState;
					BYTE tempLeftState = *ct_Player::LeftState;
					//自瞄到目标的角度
					AimBot(*Cg_Recreation::point);
					//修正仰俯角
					*ct_Player::Pitch = 0.0f;
					//准备变量
					//清零向量，让函数重新计算
					*ct_Player::vectorX = 0;
					*ct_Player::vectorY = 0;
					*ct_Player::LeftState = 0;//左右移动会影响速度
					*ct_Player::AdvanceState = 1;//强制前进
					for (int i = 0; i < 100; ++i) o_ModifyVec(player, v2, v3, v4, v5);//重新计算向量
					//还原变量
					*ct_Player::LeftState = tempLeftState;
					*ct_Player::AdvanceState = tempAdvanceState;
					*ct_Player::Pitch = tempPitch;
					*ct_Player::Yaw = tempYaw;
				}
			}
			else if (tempSpeed != -1) {//没移动复原加速状态
				Cg_Move::Speed = tempSpeed;
				Cg_Func::H_Speed = tempSpeedState;
				tempSpeed = -1;
			}

		}

		//灵活控制人物功能处理↓
		//没执行旋转我们才单独处理灵活控制功能
		if (!IsRotate && Cg_Func::H_AbsoluteControl) {
			//只有在移动的时候才进行处理
			if (*ct_Player::AdvanceState != 0 || *ct_Player::LeftState != 0) {
				o_ModifyVec(player, v2, v3, v4, v5);//先计算一下正常流程
				float temp = *ct_Player::vectorZ;//记录原值
				*ct_Player::vectorX = 0;
				*ct_Player::vectorY = 0;
				for (int i = 0; i < 100; ++i) o_ModifyVec(player, v2, v3, v4, v5);//重新计算向量
				*ct_Player::vectorZ = temp;

			}
			//不用返回，因为后面还需要处理自动跳跃
		}

		//无移动后摇功能处理↓
		if (Cg_Func::H_MoveRecovery) {
			//当我们停止行动时
			if (*ct_Player::AdvanceState == 0 && *ct_Player::LeftState == 0) {
				//将向量赋值0
				*ct_Player::vectorX = 0;
				*ct_Player::vectorY = 0;
				if (Cg_Func::H_Fly) *ct_Player::vectorZ = 0;
			}
		}

		//自动跳跃功能处理↓
		if (Cg_Func::H_AutoJump) {
			o_ModifyVec(player, v2, v3, v4, v5);//调用原函数，然后再修改z向量
			if (*ct_Player::AdvanceState == 0 && *ct_Player::LeftState == 0) return;//未移动返回
			if (Cg_Func::H_Fly) return;//飞行模式返回
			if (*ct_Player::state != 0) return;//非存活状态返回
			*ct_Player::vectorZ = Cg_Move::High;//移动并且非飞行模式并且存活才跳跃
			return;
		}

		return o_ModifyVec(player, v2, v3, v4, v5);//没开功能就走正常流程
	}
	//加速
	bool HookMove(void* player, Vector3* vec, void* p3) {
		//是否是本地玩家
		bool IsLocal = player == EntityManager.LocalPlayer.EntityPtr;

		//非本地人物处理↓
		if (!IsLocal) {
			return o_Move(player, vec, p3);//非本地人物走正常流程
		}

		//本地人物处理↓
		//加速功能处理↓
		if (Cg_Func::H_Speed) {
			Vector3* vel = (Vector3*)(ct_Player::vectorX);
			//根据汇编代码得到以下公式↓，直接修改速度值会被服务器踢
			vec->x = vel->x * Cg_Move::Speed / 2000.0f;
			vec->y = vel->y * Cg_Move::Speed / 2000.0f;
			//只有在飞行模式下才对z向量加速
			if (*ct_Player::state != 0 || Cg_Func::H_Fly) vec->z = vel->z * Cg_Move::Speed / 2000.0f;
			return o_Move(player, vec, p3);//修改完后执行函数，返回
		}

		return o_Move(player, vec, p3);//没开功能就走正常流程
	}

}

//普通功能
void AimBot(Vector3& targetPos, int SmoothLevel) {
	Vector3 myPos = EntityManager.LocalPlayer.worldPos;
	//计算位置差
	Vector3 delta;
	delta.x = targetPos.x - myPos.x;
	delta.y = targetPos.y - myPos.y;
	delta.z = targetPos.z - myPos.z;
	//计算水平距离
	float dist2D = sqrt(delta.x * delta.x + delta.y * delta.y);
	float yaw = ((atan2(delta.x, delta.y) * 180.0) / 3.1415926) * -1;
	float pitch = (atan2(delta.z, dist2D) * 180.0) / 3.1415926;

	if (SmoothLevel > 0) {//平滑
		float deltaYaw = yaw - *ct_Player::Yaw;
		float deltaPitch = pitch - *ct_Player::Pitch;
		deltaYaw = NormalizeAngle(deltaYaw);
		yaw = *ct_Player::Yaw + deltaYaw / SmoothLevel;
		pitch = *ct_Player::Pitch + deltaPitch / SmoothLevel;
		yaw = NormalizeAngle(yaw);
		*ct_Player::Yaw = yaw;
		*ct_Player::Pitch = pitch;
	}
	else  {//强锁
		*ct_Player::Yaw = yaw;
		*ct_Player::Pitch = pitch;
	}
}
void AutoFire() {
	if (!EntityManager.bestTarget.isInit()) return;
	Entity& player = EntityManager.bestTarget;
	//备份worldpos_x,y,z，最后需要还原
	float tempWorldPos_x = *ct_var::worldpos_x;
	float tempWorldPos_y = *ct_var::worldpos_y;
	float tempWorldPos_z = *ct_var::worldpos_z;

	//开启了子弹追踪功能，就用子弹追踪计算的落点来进行可视化判断
	if (Cg_Func::H_BulletTrace) LandingPointCal(player.worldPos);//计算射线落点

	//如果没有开启子弹追踪功能就用我们原本的worldpos_x,y,z来进行可视化判断
	if(CheckEntityVisibility(player)) {
		*ct_Player::fire_state = true;//设置开火状态
		Vector3* firepos = Cg_Func::H_BulletTrace ? (Vector3*)&player.worldPos : (Vector3*)ct_var::worldpos_x;
		func::Shoot(EntityManager.LocalPlayer.EntityPtr, firepos);
		*ct_Player::fire_state = false;
	}

	//还原worldpos_x,y,z
	*ct_var::worldpos_x = tempWorldPos_x;
	*ct_var::worldpos_y = tempWorldPos_y;
	*ct_var::worldpos_z = tempWorldPos_z;
}

// 辅助函数
float NormalizeAngle(float angle) {
	while (angle > 180.0f) angle -= 360.0f;
	while (angle < -180.0f) angle += 360.0f;
	return angle;
}
bool CheckEntityVisibility(Entity& entity) {
	Vector3 rayStart(*ct_Player::x, *ct_Player::y, *ct_Player::z);//1.射线起点
	Vector3 rayEnd(*ct_var::worldpos_x, *ct_var::worldpos_y, *ct_var::worldpos_z);//2.射线落点
	Vector3 playerFeet(entity.worldPos);//3.实体脚部坐标
	Vector3 playerHead = playerFeet;//4.实体头部坐标
	float hit_range = *(float*)((uintptr_t)entity.EntityPtr + ct_offsets::hitRange);//5.实体碰撞范围？
	static float* outVal = new float{};//6.负责接收函数输出的值，没研究出来含义，无用
	//修正脚部坐标，观察汇编代码
	//发现此参数需要减人物结构体+axis_z_sub偏移下的浮点数，此浮点数通常为14。猜测是人物身高
	playerFeet.z -= *(float*)((uintptr_t)entity.EntityPtr + ct_offsets::axis_z_sub);
	//射线检测，返回可见状态
	return func::IntersectRay(&rayStart, &rayEnd, &playerFeet, &playerHead, hit_range, outVal);
}
void LandingPointCal(Vector3& worldpos) {
	//八叉树根节点如果为空就不要调用func::BulletTrace
	//我发现在多人游戏切换地图时，调用func::BulletTrace();会让游戏崩溃
	//逆向分析，发现是访问了空指针导致的↓
	if (*ct_var::octree == 0) return;
	//摄像机pitch和yaw数值与人物的pitch和yaw一致，记录一份即可
	float tempPitch = *ct_Player::Pitch;
	float tempYaw = *ct_Player::Yaw;
	static uintptr_t tempCam{};
	AimBot(worldpos);//修改朝向
	//如果是第三人称我们需要修改摄像机
	//第三人称和第一人称计算射线使用的变量略微不同
	if (*ct_var::thirdPerson == true) {
		tempCam = *ct_var::camera;
		*ct_var::camera = (uintptr_t)EntityManager.LocalPlayer.EntityPtr;
	}
	//第三人称函数内会用摄像机指针下的yaw和pitch，第一人称使用人物指针下的yaw和pitch
	//我们让摄像机指针暂时指向我们的人物指针即可（偏移量一致，不然替换指针游戏直接爆炸了）
	func::BulletTrace();//通过此函数计算射线落点，修改worldpos_x,y,z
	if (*ct_var::thirdPerson == true) {
		*ct_var::camera = tempCam;//恢复指针
	}
	*ct_Player::Pitch = tempPitch;//恢复朝向
	*ct_Player::Yaw = tempYaw;

}

//子弹追踪，本项目写的第一个功能。
//现在已经靠hookshoot函数实现了
//旧艺术，不舍得删了
//注释掉HookShoot函数里的子弹追踪分支，使用Enable函数启动功能也是可以的
v2 o_bulletTrace = nullptr;
void __cdecl bulletTrace::HookFunc() {
	Entity& enemy = EntityManager.bestTarget;//取得最佳目标实体引用，EntityCache类的UpdateBestTarget函数会更新这个实体
	if (!enemy.isInit()) return;//没初始化，代表执行了空构造。意味着没有最佳目标，直接返回
	//直接修改子弹落点到敌人的坐标上
	*ct_var::worldpos_x = enemy.worldPos.x;
	*ct_var::worldpos_y = enemy.worldPos.y;
	*ct_var::worldpos_z = enemy.worldPos.z;

	return;
}
void bulletTrace::Enable() {
	if (m_enabled) return;

	o_bulletTrace = (v2)((uintptr_t)ct_baseAddr::gameBase + fc_offsets::func_bulletTrace);  // 函数头地址
	m_hook.address = (uintptr_t)((uintptr_t)ct_baseAddr::gameBase + ct_offsets::call_bulletTrace);  // call指令地址
	size_t act_len{};
	blank = blank ? blank : HookManager::FindBlankMem(m_hook.address, 10, act_len);
	if (!HookManager::Bytes5ToJmp(m_hook.address, reinterpret_cast<uintptr_t>(blank))) return;

	if (!HookManager::Bytes5ToCall((uintptr_t)blank, reinterpret_cast<uintptr_t>(o_bulletTrace))) return;

	blank1 = blank1 ? blank1 : HookManager::FindBlankMem((uintptr_t)blank + 5, 17, act_len);
	if (!blank1) return;
	if (!HookManager::Bytes5ToJmp((uintptr_t)blank + 5, reinterpret_cast<uintptr_t>(blank1))) return;

	m_hook.isActive = HookManager::InstallCallHook(//12
		(uintptr_t)blank1,
		(uintptr_t)HookFunc,
		m_hook.originalBytes,
		&m_hook.patchSize
	);

	if (!HookManager::Bytes5ToJmp(((uintptr_t)blank1 + m_hook.patchSize), m_hook.address + 5)) return;

	m_enabled = m_hook.isActive = true;
}
void bulletTrace::Disable() {
	if (!m_enabled) return;
	//MH_DisableHook(g_bulletTrace);
	if (!HookManager::Bytes5ToCall(m_hook.address, (uintptr_t)o_bulletTrace)) return;
	m_hook.isActive = false;
	m_enabled = false;
}