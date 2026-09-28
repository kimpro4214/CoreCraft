#include "pch.h"
#include "MemoryTracker.h"
#include <malloc.h>
#include <psapi.h>

namespace
{
	// 상수 초기화되므로 정적 초기화 순서와 무관하게 operator new에서 바로 사용 가능
	atomic<int64> g_currentBytes = 0;
	atomic<int64> g_peakBytes = 0;
	atomic<int64> g_liveAllocations = 0;
	atomic<int64> g_frameAllocations = 0;

	void OnAllocate(void* pointer)
	{
		const int64 size = static_cast<int64>(_msize(pointer));
		const int64 current = g_currentBytes.fetch_add(size, memory_order_relaxed) + size;
		g_liveAllocations.fetch_add(1, memory_order_relaxed);
		g_frameAllocations.fetch_add(1, memory_order_relaxed);

		int64 peak = g_peakBytes.load(memory_order_relaxed);
		while (current > peak && !g_peakBytes.compare_exchange_weak(peak, current, memory_order_relaxed)) {}
	}

	void OnFree(void* pointer)
	{
		g_currentBytes.fetch_sub(static_cast<int64>(_msize(pointer)), memory_order_relaxed);
		g_liveAllocations.fetch_sub(1, memory_order_relaxed);
	}

	constexpr double ToMB(int64 bytes) { return static_cast<double>(bytes) / (1024.0 * 1024.0); }
}

// new[]/delete[]와 sized delete는 CRT 기본 구현이 아래 함수로 포워딩한다
void* operator new(size_t size)
{
	void* pointer = malloc(size == 0 ? 1 : size);
	if (pointer == nullptr)
		throw bad_alloc();
	OnAllocate(pointer);
	return pointer;
}

void* operator new(size_t size, const nothrow_t&) noexcept
{
	void* pointer = malloc(size == 0 ? 1 : size);
	if (pointer)
		OnAllocate(pointer);
	return pointer;
}

void operator delete(void* pointer) noexcept
{
	if (pointer == nullptr)
		return;
	OnFree(pointer);
	free(pointer);
}

void operator delete(void* pointer, size_t) noexcept
{
	operator delete(pointer);
}

MemoryStats MemoryTracker::GetStats() const
{
	MemoryStats stats;
	stats.currentBytes = g_currentBytes.load(memory_order_relaxed);
	stats.peakBytes = g_peakBytes.load(memory_order_relaxed);
	stats.liveAllocations = g_liveAllocations.load(memory_order_relaxed);
	stats.frameAllocations = g_frameAllocations.load(memory_order_relaxed);
	return stats;
}

void MemoryTracker::DrawOverlay()
{
	// 프레임당 할당 횟수는 오버레이 표시 여부와 관계없이 매 프레임 리셋
	const int64 frameAllocations = g_frameAllocations.exchange(0, memory_order_relaxed);
	if (!_overlayVisible)
		return;

	const MemoryStats stats = GetStats();
	PROCESS_MEMORY_COUNTERS_EX process = {};
	K32GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&process), sizeof(process));

	constexpr float padding = 10.f;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - padding, viewport->WorkPos.y + padding), ImGuiCond_Always, ImVec2(1.f, 0.f));
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.35f);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs
		| ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;
	if (ImGui::Begin("##StatMemory", nullptr, flags))
	{
		ImGui::TextUnformatted("stat memory");
		ImGui::Separator();
		ImGui::Text("Heap Current : %8.2f MB", ToMB(stats.currentBytes));
		ImGui::Text("Heap Peak    : %8.2f MB", ToMB(stats.peakBytes));
		ImGui::Text("Live Allocs  : %lld", stats.liveAllocations);
		ImGui::Text("Allocs/Frame : %lld", frameAllocations);
		ImGui::Separator();
		ImGui::Text("Process Private : %8.2f MB", ToMB(static_cast<int64>(process.PrivateUsage)));
	}
	ImGui::End();
}
