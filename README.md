# Core Craft Engine

DirectX 11 기반 커스텀 게임 엔진 

## 개요

Unreal Engine 아키텍처(UObject → AActor → UActorComponent)에서 영감을 받아 설계된 C++/DX11 게임 엔진입니다.
컴포넌트 패턴, 스마트 포인터 기반 리소스 관리, 단계별 조명 셰이더 시스템을 갖추고 있습니다.

## 기술 스택

- **언어**: C++20
- **그래픽스 API**: DirectX 11 (Feature Level 11.0)
- **수학 라이브러리**: DirectXMath / SimpleMath
- **텍스처 로딩**: DirectXTex
- **리소스 관리**: Microsoft::WRL::ComPtr, std::shared_ptr / std::unique_ptr
- **빌드**: Visual Studio 2022, Windows SDK 10.0+

## 프로젝트 구조

```
GameCoding/
├── Engine/                         # 엔진 코어
│   ├── Game.h/cpp                  # 메인 게임 루프
│   ├── GameObject.h/cpp            # 액터 기반 게임 오브젝트
│   ├── Component.h/cpp             # 컴포넌트 베이스 클래스
│   ├── MonoBehaviour.h/cpp         # 스크립트 가능 컴포넌트
│   ├── Transform.h/cpp             # 계층적 트랜스폼
│   ├── Camera.h/cpp                # 카메라 컴포넌트 (View/Proj 행렬)
│   ├── Mesh.h/cpp                  # 메시 데이터 컴포넌트
│   ├── MeshRenderer.h/cpp          # 렌더링 컴포넌트 (Technique/Pass 기반)
│   ├── Material.h/cpp              # 머티리얼 (셰이더 + 텍스처 조합)
│   ├── Frustum.h/cpp               # 뷰 프러스텀 컬링
│   │
│   ├── Graphics.h/cpp              # DX11 디바이스, 스왑체인, DSV
│   ├── RenderManager.h/cpp         # GlobalDesc / TransformDesc / Light / Material 상수버퍼 관리
│   ├── Shader.h/cpp                # Technique / Pass 셰이더 파이프라인
│   ├── Pass.h/cpp                  # 단일 렌더 패스 (VS + PS + 상태)
│   ├── Technique.h/cpp             # 셰이더 기법 묶음
│   │
│   ├── VertexBuffer.h/cpp          # GPU 버텍스 버퍼
│   ├── IndexBuffer.h/cpp           # GPU 인덱스 버퍼
│   ├── ConstantBuffer.h            # 동적 상수 버퍼 (템플릿)
│   ├── Geometry.h                  # CPU 측 지오메트리 데이터 (템플릿)
│   ├── GeometryHelper.h/cpp        # 지오메트리 생성 유틸리티
│   ├── VertexData.h/cpp            # 버텍스 구조체 정의
│   │
│   ├── Texture.h/cpp               # 텍스처 로딩 & 바인딩
│   ├── ResourceBase.h/cpp          # 리소스 베이스 클래스
│   ├── ResourceManager.h/cpp       # 리소스 캐싱 & 관리
│   │
│   ├── InputManager.h/cpp          # 키보드/마우스 입력
│   ├── TimeManager.h/cpp           # 델타타임 & FPS
│   ├── Types.h                     # 타입 별칭 (Vec3, Matrix 등)
│   ├── Define.h                    # 공통 매크로 & 전처리기
│   └── Utils.h/cpp                 # 유틸리티 함수
│
├── Client/                         # 데모 씬 (학습/테스트용)
│   ├── 01. TriangleDemo            # 기본 삼각형 렌더링
│   ├── 02. QuadDemo                # 사각형 렌더링
│   ├── 03. ConstBufferDemo         # 상수 버퍼 활용
│   ├── 04. CameraDemo              # 카메라 이동 및 시점 제어
│   ├── 05. TextureDemo             # 텍스처 매핑
│   ├── 06. SamplerDemo             # 샘플러 스테이트 비교
│   ├── 07. HeightMapDemo           # 하이트맵 지형
│   ├── 08. NormalDemo              # 노말 벡터 시각화
│   ├── 09. MeshDemo                # Mesh 컴포넌트 기반 렌더링
│   ├── 10. GlobalTestDemo          # Global 셰이더 상수버퍼 테스트
│   ├── 11. DepthStencilDemo        # 깊이/스텐실 테스트
│   ├── 12. AmbientDemo             # 환경광 조명
│   ├── 13. DiffuseDemo             # 난반사 조명
│   ├── 14. SpecularDemo            # 정반사 조명
│   ├── 15. EmissiveDemo            # 자체 발광
│   ├── 16. LightingDemo            # 통합 조명 모델
│   ├── 17. MaterialDemo            # 머티리얼 시스템
│   ├── 18. NormalMappingDemo       # 노말 매핑
│   └── CameraScript.h/cpp          # 1인칭 카메라 스크립트
│
├── Shaders/                        # HLSL 셰이더 (.fx)
│   ├── 00. Global.fx               # 전역 상수버퍼 (GlobalDesc / TransformDesc)
│   ├── 00. Light.fx                # 조명 구조체 & 계산 함수
│   ├── 01~08. *.fx                 # 기초 셰이더 (Triangle ~ GlobalTest)
│   └── 09~13. Lighting_*.fx        # 조명 단계별 셰이더 (Ambient → Lighting)
│
└── Resources/                      # 텍스처, 리소스 파일
```

## 아키텍처

### 렌더링 흐름

```
Game::Render()
  ├─ Graphics::RenderBegin()         # RTV / DSV 클리어, 뷰포트 설정
  ├─ RenderManager::PushGlobalData() # 카메라 VP 행렬, 시간 등 전역 데이터 업데이트
  ├─ GameObject::Render()
  │   └─ MeshRenderer::Render()
  │       ├─ Technique → Pass 순회
  │       ├─ IASetPrimitiveTopology  # TRIANGLELIST (누락 시 GPU마다 다르게 동작)
  │       ├─ IASetVertexBuffers / IASetIndexBuffer
  │       ├─ VS / PS SetShader
  │       ├─ SetConstantBuffer (GlobalDesc, TransformDesc, LightDesc, MaterialDesc)
  │       ├─ SetTexture / SetSamplerState
  │       └─ DrawIndexed()
  └─ Graphics::RenderEnd()           # SwapChain Present
```

### 셰이더 상수버퍼 구조

| 버퍼 | 슬롯 | 내용 |
|------|------|------|
| GlobalDesc | b0 | VP 행렬, 카메라 위치, 경과 시간 |
| TransformDesc | b1 | World 행렬 |
| LightDesc | b2 | 방향, 색상, 조명 타입 |
| MaterialDesc | b3 | Ambient/Diffuse/Specular/Emissive 계수 |

### 주요 설계 원칙

- **컴포넌트 패턴**: `Component` 베이스 클래스를 통한 확장 가능한 게임 오브젝트
- **스마트 포인터**: DX11 리소스는 `ComPtr`, 일반 객체는 `shared_ptr` / `unique_ptr`
- **Technique / Pass 구조**: 하나의 셰이더가 여러 Pass를 가지며, MeshRenderer가 순회하며 실행
- **템플릿 버퍼**: `Geometry<T>`, `ConstantBuffer<T>` 로 타입 안전 버퍼 처리
- **데이터 지역성**: 오픈 월드 최적화를 위한 `std::vector` 기반 연속 메모리

## 트러블슈팅 (해결된 주요 이슈)

### 1. `worldPosition`이 왜곡되어 조명 계산 오류
- **원인**: VS에서 `worldPosition`을 VP 변환 **이후**에 저장
- **해결**: W 변환 직후, VP 변환 전에 저장
```hlsl
output.position = mul(input.position, W);
output.worldPosition = output.position.xyz; // 반드시 여기서
output.position = mul(output.position, VP);
```

### 2. `CameraPosition` 계산이 잘못되어 정반사 방향 오류
- **원인**: View 행렬의 이동 성분(`-V._41_42_43`)을 그대로 사용 → 월드 좌표 아님
- **해결**: View 행렬의 회전 성분(3x3)으로 역변환
```hlsl
float3 cameraPos = mul(float3(-V._41, -V._42, -V._43), (float3x3)V);
```

### 3. 일부 GPU에서 메시가 점/선으로 렌더링됨
- **원인**: `IASetPrimitiveTopology` 호출 누락 → GPU 기본값이 POINT 또는 LINE인 경우 존재
- **해결**: `MeshRenderer::Render()` 진입 시 항상 명시적으로 설정
```cpp
DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
```

### 4. `D3D11CreateDeviceAndSwapChain` 후 셰이더 기능 제한
- **원인**: Feature Level 미명시로 DX9/10 레벨로 생성될 수 있음
- **해결**: Feature Level 배열에 `D3D_FEATURE_LEVEL_11_0` 명시

### 5. 깊이 테스트 미적용으로 원근 렌더링 오류
- **원인**: `DepthStencilView` 미생성 / RTV에 DSV 미바인딩
- **해결**: `Graphics` 초기화 시 DSV 생성 후 `OMSetRenderTargets`에 함께 전달

## 브랜치 전략

- `main` — 안정 릴리즈 브랜치 (직접 push 금지)
- `dev` — 개발 브랜치 (모든 작업은 여기서)
- `dev → main` 머지는 GitHub PR을 통해서만 진행

## 빌드 환경

- Visual Studio 2022
- Windows SDK 10.0+
- DirectX 11 SDK (Windows SDK 포함)
- C++20
