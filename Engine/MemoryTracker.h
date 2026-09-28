#pragma once

// 전역 operator new/delete 교체로 C++ 힙 할당량을 추적한다.
// 한계: malloc 직접 호출, aligned new, D3D 드라이버/DLL 내부 힙은 추적하지 않는다.
struct MemoryStats
{
	int64 currentBytes = 0;
	int64 peakBytes = 0;
	int64 liveAllocations = 0;
	int64 frameAllocations = 0;
};

class MemoryTracker
{
	DECLARE_SINGLE(MemoryTracker);
public:
	MemoryStats GetStats() const;

	// stat memory 오버레이 (콘솔 "stat memory"로 토글)
	void DrawOverlay();
	void ToggleOverlay() { _overlayVisible = !_overlayVisible; }
	void SetOverlayVisible(bool visible) { _overlayVisible = visible; }
	bool IsOverlayVisible() const { return _overlayVisible; }

private:
	bool _overlayVisible = false;
};
