# BUG.md — 버그 수정 기록

---

## [1] Material/메시가 점(Point)으로 렌더링되는 버그

**증상:** 구(Sphere) 등 메시가 삼각형 면으로 렌더링되지 않고 점 점 점으로 보임. 하드웨어마다 다르게 나타남.

**원인 1 — `IASetPrimitiveTopology` 누락** (`Engine/MeshRenderer.cpp`)
- `MeshRenderer::Update()`에서 `IASetPrimitiveTopology` 호출이 없었음
- DX11은 초기 topology 상태가 명세상 undefined → 일부 GPU는 POINTLIST로 동작

**원인 2 — `pass->Apply()` 이후 topology 초기화** (`Engine/Pass.cpp`)
- `Pass::BeginDraw()`에서 `pass->Apply()` 호출 시 일부 드라이버가 IA 상태를 리셋
- `pass->Apply()` 이전에 설정해도 덮어써짐

**수정:**
```cpp
// Engine/Pass.cpp - BeginDraw()
DC->IASetInputLayout(inputLayout.Get());
pass->Apply(0, DC.Get());
DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Apply 이후에 설정
```

---

## [2] Feature Level 미명시로 하드웨어별 동작 차이

**증상:** GPU에 따라 DX11 기능이 정상 동작하지 않을 수 있음.

**원인** (`Engine/Graphics.cpp`)
- `D3D11CreateDeviceAndSwapChain` 호출 시 `pFeatureLevels=nullptr`, `FeatureLevels=0`
- 드라이버가 임의의 Feature Level(10.0, 10.1 등)로 디바이스를 생성할 수 있음

**수정:**
```cpp
D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
::D3D11CreateDeviceAndSwapChain(..., featureLevels, 1, ...);
```

---

## [3] 셰이더 VS에서 `worldPosition`을 로컬 공간으로 저장하는 버그

**증상:** 라이팅 계산이 월드 공간이 아닌 로컬 공간 기준으로 수행됨 → 조명 방향/거리 오류.

**원인** (`Shaders/13. Lighting.fx`, `14. NormalMapping.fx`, `11. Lighting_Specular.fx`, `12. Lighting_Emissive.fx`)
- `output.worldPosition = input.position.xyz` — W 변환 전 로컬 좌표를 저장

**수정:**
```hlsl
output.position = mul(input.position, W);
output.worldPosition = output.position.xyz; // 반드시 W 변환 후, VP 변환 전
output.position = mul(output.position, VP);
```

---

## [4] `CameraPosition()` 잘못된 공식

**증상:** 스페큘러/에미시브 계산에서 카메라 방향 벡터(E)가 틀림 → 반사광 오류.

**원인** (`Shaders/00. Global.fx`, `11. Lighting_Specular.fx`, `12. Lighting_Emissive.fx`)
- `return -V._41_42_43` — 뷰 행렬의 translation만 추출하며 회전 성분 미반영

**수정:**
```hlsl
// 00. Global.fx
float3 CameraPosition()
{
    return mul(float3(-V._41, -V._42, -V._43), (float3x3)V);
}
```
- `Lighting_Specular.fx`, `Lighting_Emissive.fx`의 직접 계산도 `CameraPosition()` 호출로 교체

---

## 빌드 주의사항

`Engine/*.cpp` 수정 시 **Engine 프로젝트를 먼저 빌드**해야 `Libraries/Lib/Engine/Engine.lib`가 갱신됨.
Client만 빌드하면 이전 lib을 그대로 링크하므로 변경사항이 반영되지 않음.
