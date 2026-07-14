# Third-party libraries

에디터 전용 라이브러리는 이 폴더에 고정 버전으로 포함한다.

- Dear ImGui docking `v1.89.9-docking` (`1d8e48c`): MIT
- ImGuizmo `1.10` (`b796ac3`): MIT
- imgui-node-editor (`021aa0e`): MIT

`imgui-node-editor`의 `IM_TRUNC` 호출은 ImGui 1.89 호환을 위해 동일 의미의 `IM_FLOOR`로 패치했고, 중복 `GetKeyIndex` 호환 함수는 제거했다.

각 라이브러리의 원본 LICENSE 파일을 해당 디렉터리에 유지한다.
