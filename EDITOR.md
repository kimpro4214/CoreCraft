# Core Craft Editor MVP

DX11 엔진 위에서 씬을 제작하고 Blueprint 동작을 붙인 뒤 독립 실행 파일로 패키징하는 데모 편집기입니다.

## 실행

Visual Studio에서 `GameCoding.sln`의 시작 프로젝트를 `Editor`로 지정하고 `Debug | x64`로 실행합니다. 실행 파일의 작업 디렉터리는 `Editor` 폴더여야 셰이더와 리소스 상대 경로가 맞습니다.

## 주요 기능

- `Hierarchy`: Cube/Sphere 생성, 선택, 삭제
- `Inspector`: 이름, 활성 상태, Location/Rotation/Scale 편집
- `Viewport`: 오프스크린 DX11 렌더링과 ImGuizmo Move/Rotate/Scale
- `Content Browser`: `Resources` 아래 에셋 탐색
- `Blueprint`: 노드 생성, 실행 핀 연결/삭제, 컴파일 및 XML 저장
- `Play/Pause/Stop`: 편집 씬 스냅샷을 보존한 상태로 Blueprint 실행
- `Demo > Create Third Person Demo`: Kachujin 플레이어, 카메라, 바닥과 조명이 포함된 시연 씬 생성
- `File`: 새 씬, XML 씬 열기, 저장, 다른 이름으로 저장
- `Build > Package Project`: Release 런타임과 시작 씬을 `Builds/CoreCraft`에 생성

창 배치가 흐트러지면 `Window > Reset Layout`을 사용합니다.

## 뷰포트 조작

- 마우스 오른쪽 버튼을 누른 상태에서 마우스 이동: 카메라 회전
- 마우스 오른쪽 버튼 + `W/A/S/D`: 카메라 이동
- Toolbar의 `Move/Rotate/Scale`: Transform 기즈모 모드 전환
- `W/E/R`: Move/Rotate/Scale 기즈모 전환
- Viewport 좌클릭: 메시 또는 조명 선택
- Play 중 `W/A/S/D`: 플레이어 이동
- Play 중 마우스 오른쪽 버튼 드래그: 3인칭 카메라 회전
- Play 중 `Space`: 점프

## 파일 형식

- 씬: `Resources/Scenes/*.scene.xml`
- Blueprint: `Resources/Blueprints/*.blueprint.xml`
- 샘플 시작 씬: `Resources/Scenes/Startup.scene.xml`

패키지 실행 파일은 `Builds/CoreCraft/Binaries/GameRuntime.exe`입니다. 패키징된 실행 파일은 ImGui 편집기 코드를 표시하지 않고 `Startup.scene.xml`만 로드합니다.

## 현재 MVP 범위

씬 계층, Transform, MeshRenderer, AnimatedModel, Camera, Directional/Point Light, PlayerController, Blueprint 참조를 저장합니다. Blueprint는 BeginPlay/Tick과 Set Location/Add Offset/Add Rotation/Print Log 실행 노드를 지원합니다. 플레이어 물리는 Y=0 바닥의 중력과 점프까지만 지원합니다. 에셋 임포트, 범용 충돌/물리, 그림자, Cook 파이프라인은 후속 범위입니다.
