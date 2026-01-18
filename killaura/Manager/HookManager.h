#pragma once
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <vector>

class HookManager {
private:
    static BYTE int3;
    static BYTE WhiteMem;
public:
    static bool WriteMemory(void* dest, const void* src, size_t len);

    static bool InstallCallHook(uintptr_t hookAddr, uintptr_t targetFunc,
        uint8_t* originalBytes, size_t* patchSize);

    static bool RestoreHook(uintptr_t hookAddr, const uint8_t* originalBytes,
        size_t len);

    //查找空白内存    返回空白内存的首地址
    //1.开始地址    2.需要的空白长度   3.实际空白长度
    static void* FindBlankMem(uintptr_t strAddr, size_t blank_len, size_t& act_len);

    //将5字节的指令转换为Jmp相对跳转
    //1.指令地址    2.需要跳转到的地址(与指令地址偏移不得超过INT_MAX!)
    static bool Bytes5ToJmp(uintptr_t o_tar, uintptr_t p_target);

    //将5字节的指令转换为call相对跳转
    //1.指令地址    2.需要跳转到的地址(与指令地址偏移不得超过INT_MAX!)
    static bool Bytes5ToCall(uintptr_t o_tar, uintptr_t p_target);
public:
    //写入内存补丁
    //1.补丁内容    2.写入的地址    3.补丁长度
    static void WriteMemoryPatch(BYTE* replace, BYTE* addr, size_t len);
};

//内存补丁管理类
class MemoryPatch {
private:
    void* _targetAddr;//补丁写入地址
    std::vector<BYTE> _originalBytes;//原本的机器码
    std::vector<BYTE> _patchBytes;//补丁内容
    bool _isApplied = false;//补丁是否应用

public:
    //构造函数  会自动记录原本机器码
    //1.写入地址    2.补丁内容
    MemoryPatch(uintptr_t targetAddr, std::vector<BYTE> patchBytes);

    //应用补丁
    void Apply();

    //取消应用补丁
    void Restore();

    //返回补丁地址
    uintptr_t PatchAddr()const { return (uintptr_t)_targetAddr; }

    //返回补丁应用状态
    bool IsApplied() const { return _isApplied; }
};

// ==================== Hook信息结构 ====================
struct HookInfo {
    uintptr_t address = 0;       // 要hook/修改的目标地址
    uint8_t originalBytes[16] = { 0 };  // 保存原始字节
    size_t patchSize = 0;        // patch的大小
    bool isActive = false;       // hook是否激活状态
};
