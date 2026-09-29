# Core Craft 엔진 로드맵

언리얼 스타일 `UObject`/`AActor`/컴포넌트 계층을 실제로 써보면서 확인한, 앞으로 채워야 할 구멍들.

## AssetManager

`Model`이 `ResourceBase`를 상속하지 않아 `ResourceManager`의 캐시(`Load<T>`/`Get<T>`)를 타지 않는다.
`StaticMeshDemo`/`ObjViewerDemo` 같은 각 데모가 `make_shared<Model>()` + `ReadModel/ReadMaterial`을
직접 호출하는 수동 패턴이라, 같은 모델을 여러 액터가 쓰면 그만큼 중복 로드된다. `Model`도
`ResourceBase` 계열로 편입하거나, `Model` 전용 캐시를 가진 매니저가 필요하다.

## 물리 / 충돌

충돌체(콜라이더)나 리지드바디 개념이 아예 없다. `USceneComponent`/`UPrimitiveComponent`에 바운드
볼륨이나 충돌 채널 같은 최소한의 훅조차 없는 상태.

## 텍스처 업로드

`Texture::Load`가 `LoadFromWICFile`만 써서 PNG/JPG류만 읽는다. `AssimpTool/Converter::WriteTexture`는
임베디드 텍스처를 DDS로 저장까지 하는데 정작 엔진이 DDS를 로드하는 경로가 없다. 비동기/스트리밍
업로드도 없어서 큰 텍스처를 로드하면 그 프레임이 그대로 멈춘다.

## 이벤트 채널

컴포넌트/액터 간 통신이 `GetXXX()`로 직접 캐스팅해서 참조를 얻는 방식뿐이다(예:
`AActor::GetComponentByClass<T>()`). 디커플링된 메시지/이벤트 버스가 없어서, 서로 모르는 시스템끼리
느슨하게 통신할 방법이 없다.

## 가비지 컬렉터

`UObject` 트리는 전부 `shared_ptr`/`weak_ptr` 수동 관리다. `USceneComponent`의 부모-자식은
`weak_ptr`로 순환 참조를 피했지만, 이건 그 클래스 하나의 설계일 뿐 전역적으로 강제되는 정책이
아니다. 새 컴포넌트를 만들 때 실수로 `shared_ptr`끼리 순환을 만들면 그대로 누수된다. 언리얼처럼
명시적인 GC 패스(또는 최소한 순환 참조를 감지하는 디버그 도구)가 없다.

## 폰트 렌더링 (텍스처 아틀라스 / MSDF)

인게임 텍스트(HUD, 대미지 숫자, 네임플레이트 등)를 그릴 방법이 전혀 없다(ImGui 텍스트는 에디터
전용). 비트맵 폰트 아틀라스나 MSDF(Multi-channel Signed Distance Field) 방식으로 폰트를 텍스처에
구워 넣고, 글자마다 쿼드를 배치해서 그리는 텍스트 렌더링 시스템이 필요하다.

## 빌보드 렌더링 / Sub UV

카메라를 항상 바라보는 빌보드 쿼드 렌더링이 없다. 파티클/이펙트에 필수인 스프라이트 시트 애니메이션
(Sub UV — 한 텍스처를 격자로 나눠 프레임별로 잘라 쓰는 것)도 마찬가지로 없다.

## UV 스크롤

머티리얼의 텍스처 UV를 시간에 따라 흘러가게 하는 스크롤 기능이 없다(물, 용암, 컨베이어 벨트 등에
필요). `MaterialDesc`에 스크롤 속도를 추가하고 셰이더에서 시간 기반 UV 오프셋을 적용해야 한다.

## 4분할 뷰포트

에디터 뷰포트가 카메라 하나짜리 단일 뷰(`EditorApp::_viewportTarget`)뿐이다. 언리얼 에디터처럼
Top/Front/Side(직교) + Perspective 4분할 뷰를 보여주려면 렌더타겟과 카메라를 뷰포트별로 분리해야
한다(직교 투영 카메라는 `Camera::SetProjectionType`으로 지원됨).

## 윈도우 리사이즈 대응

런타임에 창 크기를 바꿨을 때 스왑체인/뎁스버퍼/뷰포트를 다시 만드는 리사이즈 처리가 필요하다.

## 씬(월드) 매니저 확장

`SceneManager`는 활성 씬 하나만 관리한다(`GetActiveScene`) — 다중 월드나 레벨 스트리밍 개념이 없다.
새로 만든 `UWorld`도 마찬가지로 각 데모가 직접 `make_shared<UWorld>()`로 만들 뿐, 여러 월드를
중앙에서 관리하는 매니저가 없다.

## 에디터 아웃라이너 통합

`AssimpTool/ObjViewerDemo`에 만든 아웃라이너는 그 데모 전용이고, 실제 프로덕션 `Editor`
(GameObject 기반)에는 붙어 있지 않다. `UWorld`/`AActor` 계층을 `Editor`에 통합하고 아웃라이너를
정식 에디터 패널로 승격하는 작업이 필요하다.

## OBJ 머티리얼(.mtl) 전체 파싱

`AssimpTool/Converter::ReadMaterialData`가 Assimp의 `AI_MATKEY_COLOR_*`/텍스처 정도만 다루는
것으로 보인다. `.mtl`의 Kd(diffuse)/Ka(ambient)/Ks(specular)/Ns(shininess) 등 머티리얼 값을
빠짐없이 읽어서 `.mesh`/`.xml`에 저장하도록 커버리지를 넓혀야 한다(정확히 어떤 필드가 지금
빠져 있는지는 재확인 필요).

## 드로우콜 최적화 (정렬 / 배치)

`Scene::Render`가 오브젝트 순서대로 `MeshRenderer::Render`를 호출해서, 오브젝트마다
머티리얼 업데이트 → 트랜스폼 상수 버퍼 갱신 → VB/IB 바인딩 → `DrawIndexed`가 한 번씩 일어난다.
셰이더/머티리얼/메시 기준 정렬이 없어서 State Change가 오브젝트 수만큼 반복된다. 렌더 큐에
드로우 항목을 모은 뒤 정렬 키(셰이더 → 머티리얼 → 메시)로 정렬해 같은 상태를 연속으로 그리도록
바꿔야 한다. 효과를 수치로 보기 위해 `stat memory`처럼 드로우콜/State Change 횟수를 보여주는
통계 오버레이(`stat rendering` 등)를 먼저 만드는 것이 좋다.

## GPU 인스턴싱

`Shader`/`Technique`/`Pass`에 `DrawIndexedInstanced` 래퍼와 `INST*` 시맨틱을 슬롯 1
per-instance로 잡는 입력 레이아웃 처리(`Shader.cpp`)는 있지만, 실제로 쓰는 곳이 없다. 셰이더에
`INST` 입력이 없고 인스턴스 버퍼 클래스도 없다. 같은 메시+머티리얼 오브젝트들의 월드 행렬을
인스턴스 버퍼(동적, 프레임마다 Map)에 모아 한 번의 `DrawIndexedInstanced`로 그려야 한다.
위 정렬/배치가 선행되면 인스턴스 그룹을 자연스럽게 묶을 수 있다.
※ `Pass::DrawIndexedInstanced`가 마지막 인자로 `startInstanceLocation` 대신
`startIndexLocation`을 넘기는 버그가 있다(현재 호출처가 없어 무해). 인스턴싱 구현 시 함께 수정.

## 프러스텀 컬링

`Scene::Render`가 모든 `GameObject`를 조건 없이 그린다. `Engine/Frustum`에 평면 6개 추출과
`ContainsSphere`가 이미 있지만 렌더 경로 어디에서도 호출되지 않는다. 카메라 VP에서 프러스텀을
갱신하고, 오브젝트의 월드 바운드(메시 AABB는 `Mesh::GetBounds()`로 준비됨)로 화면 밖 오브젝트의
드로우콜을 건너뛰어야 한다. 오픈월드 규모에서는 가장 먼저 효과가 나는 최적화다.

## 오클루전 컬링

프러스텀 안에 있어도 다른 물체에 완전히 가려진 오브젝트는 여전히 그려진다. 하드웨어 오클루전
쿼리(`D3D11_QUERY_OCCLUSION`, 프레임 지연 허용) 또는 CPU 소프트웨어 뎁스 버퍼 기반 판정 중 하나를
골라야 한다. 프러스텀 컬링과 BVH가 선행되어야 비용 대비 효과가 난다.

## BVH (공간 분할 가속 구조)

컬링과 피킹이 모두 오브젝트 전체를 선형 순회한다(`Scene::Render`, `EditorApp::PickViewportObject`).
오브젝트 수가 늘면 O(N)이 병목이 되므로, 월드 AABB 기반 BVH를 구축해 프러스텀/레이 쿼리를
O(log N)으로 줄여야 한다. 움직이는 오브젝트를 위한 갱신 전략(refit vs rebuild)도 정해야 한다.
메시 내부 삼각형 BVH는 고폴리 모델의 정밀 피킹(`RayMeshIntersect`)에도 재사용할 수 있다.

## SIMD 최적화

수학 연산이 SimpleMath(`Vec3`/`Matrix`) 스칼라 래퍼 위주라 DirectXMath의 `XMVECTOR` SIMD 경로를
거의 활용하지 못한다. 트랜스폼 계층 갱신, 컬링의 AABB-평면 판정, 레이-삼각형 판정처럼 대량으로
반복되는 핫 루프를 `XMVECTOR`/SoA 레이아웃으로 바꿔 한 번에 여러 개를 처리하도록 최적화해야 한다.
프로파일링으로 실제 핫스팟을 먼저 확인한 뒤 진행한다.
