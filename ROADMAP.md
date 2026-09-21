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
