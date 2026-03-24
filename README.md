# Core Craft Engine

DirectX 11 기반 커스텀 게임 엔진 (Palworld 스타일 오픈 월드 목표)

## 개요

Unreal Engine 아키텍처(UObject → AActor → UActorComponent)에서 영감을 받아 설계된 C++/DX11 게임 엔진입니다.

## 기술 스택

- **언어**: C++17
- **그래픽스 API**: DirectX 11
- **수학 라이브러리**: DirectXMath / SimpleMath
- **텍스처 로딩**: DirectXTex
- **리소스 관리**: Microsoft::WRL::ComPtr, std::shared_ptr

## 프로젝트 구조

```
GameCoding/
├── Core
│   ├── Game.h/cpp              # 메인 게임 루프
│   ├── GameObject.h/cpp        # 액터 기반 게임 오브젝트
│   ├── Component.h/cpp         # 컴포넌트 기반 클래스
│   └── Transform.h/cpp         # 계층적 트랜스폼 컴포넌트
│
├── Rendering
│   ├── Graphics.h/cpp          # DX11 디바이스 & 스왑체인
│   ├── Pipeline.h/cpp          # 렌더 스테이트 파이프라인
│   └── Shader.h/cpp            # 셰이더 컴파일 & 관리
│
├── Buffers
│   ├── VertexBuffer.h/cpp      # GPU 버텍스 버퍼
│   ├── IndexBuffer.h/cpp       # GPU 인덱스 버퍼
│   ├── ConstantBuffer.h        # 동적 상수 버퍼 (템플릿)
│   ├── Geometry.h              # CPU 측 지오메트리 데이터 (템플릿)
│   ├── GeometryHelper.h/cpp    # 지오메트리 생성 유틸리티
│   └── VertexData.h/cpp        # 버텍스 구조체 정의
│
├── RenderStates
│   ├── InputLayout.h/cpp       # 버텍스 입력 레이아웃
│   ├── RasterizerState.h/cpp   # 래스터라이저 설정
│   ├── BlendState.h/cpp        # 블렌딩 설정
│   ├── SamplerState.h/cpp      # 텍스처 샘플링
│   └── Texture.h/cpp           # 텍스처 로딩 & 바인딩
│
├── Shaders
│   ├── Default.hlsl            # 텍스처 렌더링 셰이더
│   └── Color.hlsl              # 버텍스 컬러 셰이더
│
└── Utility
    ├── Types.h                 # 타입 별칭 (Vec3, Matrix 등)
    ├── Struct.h                # 구조체 정의
    └── Values.h                # 전역 상수
```

## 아키텍처

### 렌더링 흐름

```
Game::Render()
  ├─ Graphics::RenderBegin()     # RTV 클리어 & 뷰포트 설정
  ├─ GameObject::Render()
  │   ├─ Pipeline::UpdatePipeline()   # IA → VS → RS → PS → OM
  │   ├─ SetVertexBuffer / IndexBuffer
  │   ├─ SetConstantBuffer (TransformData)
  │   ├─ SetTexture / SetSamplerState
  │   └─ DrawIndexed()
  └─ Graphics::RenderEnd()       # SwapChain Present
```

### 주요 설계 원칙

- **컴포넌트 패턴**: `Component` 베이스 클래스를 통한 확장 가능한 게임 오브젝트
- **스마트 포인터**: DX11 리소스는 `ComPtr`, 일반 객체는 `shared_ptr`/`unique_ptr`
- **파이프라인 추상화**: `Pipeline` 클래스가 DX11 스테이트 관리를 캡슐화
- **템플릿 버퍼**: `Geometry<T>`, `ConstantBuffer<T>` 로 타입 안전 버퍼 처리
- **데이터 지역성**: 오픈 월드 최적화를 위한 `std::vector` 기반 연속 메모리

## 브랜치 전략

- `main` — 안정 릴리즈 브랜치 (직접 push 금지)
- `dev` — 개발 브랜치 (모든 작업은 여기서)
- `dev → main` 머지는 GitHub PR을 통해서만 진행

## 빌드 환경

- Visual Studio 2022
- Windows SDK 10.0+
- DirectX 11 SDK (Windows SDK 포함)
