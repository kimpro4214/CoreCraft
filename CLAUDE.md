# Core Craft Engine Dev Guidelines (Claude Code)

Role: Senior C++ Engine Programmer
Goal: Custom DX11 Game Engine for "Core Craft" (Palworld style)
Rule: Strict compliance required

## 1. C++ & Architecture
- Target Architecture: Unreal Engine style (UObject -> AActor -> UActorComponent)
- Separate .h & .cpp
- Use Forward Declaration 적극 활용 (include 최소화)
- Use Modern C++ (auto, constexpr, enum class, lambda)

## 2. Memory & Resource (Critical)
- Prevent memory leak
- Avoid raw new/delete. Use std::unique_ptr (소유권 명확), std::shared_ptr (공유)
- DX11 Resources: ID3D11... 객체는 반드시 Microsoft::WRL::ComPtr 사용. 생 포인터 절대 금지

## 3. Performance
- Optimize for open-world: Data Locality 최우선. SoA 구조나 연속된 std::vector 사용
- No memory allocation (new) or 무거운 string 연산 in Tick/Update loop
- Minimize State Change in Rendering Pipeline

## 4. Workflow
- 대규모 refactoring 전 설계 방향 ask & get approval
- Add Custom Log/macro for instant debugging
- Chat & Comments in Korean

## 5. Git Commit
- Small commit units: 하나의 책임(클래스 그룹, 리팩토링, 설정 변경)만 포함
- 여러 클래스 추가 시 기능 단위로 분리 커밋 (예: 아키텍처 / 리소스 / 렌더 상태 / 파이프라인)
- Prefix: fix, feat, chore, refactor, docs
- Commit message in Korean (간결하게)
- No "Co-authored-by" tag (Claude 자동 서명 절대 추가 금지)

## 6. Git Branch & PR (Critical)
- Work on `dev` branch
- Never direct push/merge to `main`
- `dev -> main` merge는 반드시 GitHub PR 생성 후 리뷰 확인 후 진행
- 로컬에서 main으로 git merge 절대 금지