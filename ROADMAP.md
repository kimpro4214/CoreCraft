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

## 피킹 업데이트

`Editor::PickViewportObject`가 라이트는 `BoundingSphere`, 나머지는 로컬 공간 `BoundingBox`로만
판정한다(`3c9bc50` 커밋에서 정교화됐지만 여전히 바운딩 볼륨 단위). 메시 단위 정밀 판정이 없어서
울퉁불퉁한 모델일수록 피킹이 부정확하다. AABB로 먼저 걸러낸 뒤 실제 삼각형과 레이가 교차하는지
뮐러-트럼보어(Möller–Trumbore) 알고리즘으로 정밀 판정하는 단계가 필요하다.

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

## 와이어프레임 렌더 모드

래스터라이저를 solid/wireframe으로 토글하는 기능이 없다. 디버깅용으로 `D3D11_FILL_WIREFRAME`
래스터라이저 스테이트를 하나 더 만들어 런타임에 전환할 수 있어야 한다.

## UV 스크롤

머티리얼의 텍스처 UV를 시간에 따라 흘러가게 하는 스크롤 기능이 없다(물, 용암, 컨베이어 벨트 등에
필요). `MaterialDesc`에 스크롤 속도를 추가하고 셰이더에서 시간 기반 UV 오프셋을 적용해야 한다.

## 디버그 라인 렌더링

그리드/콜리전/디버그용 선을 그릴 방법이 없다. 선 하나하나를 매 프레임 개별 드로우콜로 그리기보다는,
그 프레임에 요청된 선들을 하나의 버텍스 버퍼에 모아 한 번의 드로우콜로 그리는 배치 렌더링으로
드로우콜 수를 최적화하는 방향으로 간다(픽셀 셰이더 트릭보다 버텍스 버퍼 배칭 우선 검토).

## 직교 투영 카메라

`Camera`에 `ProjectionType`(Perspective/Orthographic) enum은 이미 있지만 실제로는 쓰이지 않는다 —
`Camera::UpdateMatrix()`가 `XMMatrixPerspectiveFovLH`만 호출해서 항상 원근 투영이다. 직교 투영
분기를 추가해야 탑/사이드/프론트 같은 직교 뷰가 가능해진다.

## 4분할 뷰포트

에디터 뷰포트가 카메라 하나짜리 단일 뷰(`EditorApp::_viewportTarget`)뿐이다. 언리얼 에디터처럼
Top/Front/Side(직교) + Perspective 4분할 뷰를 보여주려면 렌더타겟과 카메라를 뷰포트별로 분리해야
한다(직교 투영 카메라가 선행 조건).

## 윈도우 리사이즈 대응

런타임에 창 크기를 바꿨을 때 스왑체인/뎁스버퍼/뷰포트를 다시 만드는 리사이즈 처리가 필요하다.

## Heap 메모리 사용량 오버레이

할당량을 추적해서 언리얼의 `stat memory`처럼 ImGui 오버레이 창으로 띄우는 기능이 없다. 커스텀
allocator나 할당 후킹으로 힙 사용량을 추적하는 작업이 선행돼야 한다.

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
