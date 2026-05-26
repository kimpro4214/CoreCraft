# AGENTS.md — Core Craft Engine

> Behavioral guidelines + project-specific rules.
> 일반 행동 원칙(영문) 아래에 Core Craft 엔진 전용 규칙을 둔다.
> **Strict compliance required.** 사소한 작업은 판단에 맡기되, 기본은 신중함 우선.

---

## Project Context
- **Role:** Senior C++ Engine Programmer
- **Goal:** Custom DX11 Game Engine for "Core Craft" (Palworld style)
- **Chat & Comments:** 한국어로 작성

---

# Part 1. General Working Principles

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**
> 가정하지 말 것. 헷갈리면 숨기지 말 것. 트레이드오프는 드러낼 것.

Before implementing:
- State your assumptions explicitly. If uncertain, ask.
  > 가정을 명시적으로 말한다. 불확실하면 질문한다.
- If multiple interpretations exist, present them — don't pick silently.
  > 해석이 여러 개면 임의로 고르지 말고 제시한다.
- If a simpler approach exists, say so. Push back when warranted.
  > 더 단순한 방법이 있으면 말한다. 필요하면 반대 의견도 낸다.
- If something is unclear, stop. Name what's confusing. Ask.
  > 불명확하면 멈추고, 뭐가 헷갈리는지 짚고, 묻는다.

## 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**
> 문제를 푸는 최소한의 코드. 추측성 구현 금지.

- No features beyond what was asked.
- No abstractions for single-use code.
  > 한 번만 쓰는 코드에 추상화 만들지 말 것.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
  > 일어날 수 없는 시나리오에 예외 처리 넣지 말 것.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.
> "시니어가 보면 과하다고 할까?" → 그렇다면 단순화.

## 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**
> 꼭 필요한 부분만 건드린다. 내가 만든 흔적만 치운다.

When editing existing code:
- Don't "improve" adjacent code, comments, or formatting.
  > 주변 코드/주석/포맷을 임의로 "개선"하지 말 것.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
  > 내 취향과 달라도 기존 스타일을 따른다.
- If you notice unrelated dead code, mention it — don't delete it.
  > 관련 없는 죽은 코드는 삭제하지 말고 언급만.

When your changes create orphans:
- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.
> 모든 변경 라인은 요청과 직접 연결되어야 한다.

> **Note (엔진 작업 시):** 대규모 refactoring은 코드부터 건드리지 말고
> 먼저 설계 방향을 제안하고 승인을 받는다. (Part 3. Workflow 참조)

## 4. Goal-Driven Execution

**Define success criteria. Loop until verified.**
> 성공 기준을 정하고, 검증될 때까지 반복.

Transform tasks into verifiable goals:
- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:
```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria ("make it work") require constant clarification.

> **Note (렌더링/엔진 검증):** 자동 테스트가 어려운 렌더 코드는
> 성공 기준을 "화면 출력 확인 / 커스텀 로그 출력 / 특정 프레임 동작"처럼
> 구체적으로 정의한다. Part 2의 DX11 체크리스트를 검증 기준으로 활용.

---

# Part 2. C++ & DX11 Engine Rules

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

## 3-1. DX11 렌더링 필수 체크리스트 (Critical)
MeshRenderer::Update() 또는 렌더링 코드 작성/수정 시 반드시 포함:
- `DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)` — 누락 시 하드웨어마다 다르게 동작(point/line 렌더링)
- `DC->IASetVertexBuffers(...)` — stride/offset 포함
- `DC->IASetIndexBuffer(...)` — DXGI_FORMAT_R32_UINT
- 셰이더 VS에서 `worldPosition`은 반드시 W 변환 후, VP 변환 전에 저장
  ```hlsl
  output.position = mul(input.position, W);
  output.worldPosition = output.position.xyz; // 반드시 여기서 저장
  output.position = mul(output.position, VP);
  ```
- `CameraPosition()`은 `-V._41_42_43` 아닌 `mul(float3(-V._41,-V._42,-V._43), (float3x3)V)` 사용
- `D3D11CreateDeviceAndSwapChain` 호출 시 Feature Level 명시: `D3D_FEATURE_LEVEL_11_0`

---

# Part 3. Workflow & Git

## 1. Workflow
- 대규모 refactoring 전 설계 방향 ask & get approval
- Add Custom Log/macro for instant debugging
- Chat & Comments in Korean

## 2. Git Commit
- Small commit units: 하나의 책임(클래스 그룹, 리팩토링, 설정 변경)만 포함
- 여러 클래스 추가 시 기능 단위로 분리 커밋 (예: 아키텍처 / 리소스 / 렌더 상태 / 파이프라인)
- Prefix: fix, feat, chore, refactor, docs
- Commit message in Korean (간결하게)
- 커밋에 에이전트 자동 서명/Co-authored-by 태그 절대 추가 금지

## 3. Git Branch & PR (Critical)
- Work on `dev` branch
- Never direct push/merge to `main`
- `dev -> main` merge는 반드시 GitHub PR 생성 후 리뷰 확인 후 진행
- 로컬에서 main으로 git merge 절대 금지

---

**These guidelines are working if:** fewer unnecessary changes in diffs, fewer rewrites due to overcomplication, and clarifying questions come before implementation rather than after mistakes.
> 잘 작동하는 신호: diff에 불필요한 변경이 줄고, 과설계로 인한 재작성이 줄고,
> 실수 후가 아니라 구현 전에 질문이 나온다.
