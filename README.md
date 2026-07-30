# FirstOrbit — 궤도 진입 (Orbit Insertion)

![C++](https://img.shields.io/badge/C%2B%2B-20-blue)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)
![Status](https://img.shields.io/badge/Status-In%20Development-yellow)
![License](https://img.shields.io/badge/License-MIT-green)

> 지표면에서 우주선을 발사해 대기권을 뚫고, 스케일을 넘나드는 카메라로 우주를 드러낸 뒤,
> 중력을 이용해 **안정 궤도에 진입**하는 물리 조종 게임.

외부 게임 엔진과 물리 라이브러리 없이, **WinAPI + GDI만으로** 행렬 카메라·수치적분·
만유인력·궤도역학을 밑바닥부터 구현하는 프로젝트입니다.

---

## 📋 프로젝트 개요

| 항목 | 내용 |
|---|---|
| 장르 | 2D 물리 시뮬레이션 / 조종 게임 |
| 플랫폼 | Windows (WinAPI, GDI 렌더링) |
| 개발 환경 | C++20 / WinAPI 전용 (외부 엔진·물리 라이브러리 미사용) |
| 엔진 베이스 | 직접 만든 자체 프레임워크 (World / AActor / GameMode / Component 구조) |
| 화면 해상도 | 480 x 800 |
| 개발 기간 | 약 3주 |
| 인원 | 1인 개발 |

## 🎯 왜 이 프로젝트인가

**"엔진·물리 라이브러리 없이, WinAPI만으로 물리를 밑바닥부터 구현한다"**를 증명하기
위한 프로젝트입니다. 그 증명에 가장 적합한 소재가 궤도역학이라고 판단했습니다 —
벡터·행렬·수치적분·보존법칙이 하나의 플레이 경험 안에 자연스럽게 녹아들고,
"스케일을 넘나드는 카메라"라는 시각적 자랑거리까지 만들어줍니다.

## ⭐ 핵심 기술 4축

| 축 | 내용 |
|---|---|
| **행렬 카메라** | 2D 동차좌표 3x3 행렬(SRT)로 뷰 변환 구현. 스케일 줌 하나로 지표면 횡스크롤 ↔ 궤도 탑뷰를 연속적으로 전환. 클릭 피킹은 역행렬로 화면 좌표를 월드로 되돌려 처리 |
| **수치 적분** | 명시적 오일러 / **반암시적 오일러** / RK4를 런타임으로 갈아끼우며 비교. 왜 심플렉틱 적분기가 궤도 게임에 적합한지(에너지 보존 vs 정확도) 직접 검증 |
| **만유인력** | `F = G·m·M / r²`, 벡터 힘 합성. 우주선만 중력을 받도록 제한해 n-body 붕괴를 회피 |
| **궤도 요소** | 위치·속도로 비에너지·각운동량을 구해 장반경·이심률 산출. 근지점(`a(1-e)`)이 대기권 위면 승리로 판정 |

## 🛠 기술 스택

- **C++20** (Visual Studio 2022, MSVC)
- **WinAPI + GDI** — 소프트웨어 렌더링
- **DirectSound** — 오디오
- **[Dear ImGui](https://github.com/ocornut/imgui)** — 디버그/개발용 UI
- **[nlohmann/json](https://github.com/nlohmann/json)** — 데이터 로딩
- 외부 게임 엔진·물리 엔진 **미사용**

## 🚧 진행 상황

현재 **1주차(행렬 카메라) 작업 중**입니다.

- [x] 프로젝트 배선 (씬 등록, 입력 처리 확장)
- [x] `Vector2` 수학 유틸리티
- [x] `Matrix3x3` — 기본 변환(이동/스케일/회전/곱셈) + 역행렬 2종(여인수 공식, 가우스-조던 소거법)
- [ ] `Camera` 뷰 행렬 통합 (줌 보간, 표면 정렬 회전, 클릭 피킹)
- [ ] 2주차 — 물리 코어 (중력, 반암시적 오일러, 발사 시퀀스)
- [ ] 3주차 — 궤도 판정 + HUD + 기술 회고

상세 일정은 [`주차별_로드맵.md`](./주차별_로드맵.md), 실시간 체크리스트는
[`docs/study/1주차_실행가이드라인.md`](./docs/study/1주차_실행가이드라인.md)를 참고하세요.

## 🚀 빌드 및 실행

1. **Visual Studio 2022** (C++ 데스크톱 개발 워크로드) 설치
2. `FirstOrbit.sln` 열기
3. 빌드 후 실행 (`Ctrl+F5`)

별도 SDK 설치가 필요 없습니다 — Windows SDK는 Visual Studio 설치 시 함께 포함됩니다.

## 📁 프로젝트 구조

```
FirstOrbit/
├─ FirstOrbit/
│  ├─ Core/             # 매니저 계층 (GameInstance, InputManager, ResourceManager, ...)
│  ├─ GameFramework/     # 엔진 계층 (World, AActor, GameMode, Camera, Component)
│  ├─ Worlds/            # 씬 (Title, Main, GameOver, Editor)
│  ├─ ImGui/, Json/      # 외부 라이브러리
│  └─ Structs.h          # Vector2, Matrix3x3 등 수학 유틸리티
├─ Resources/            # 이미지 · 사운드 · 폰트 · 데이터(JSON)
└─ docs/study/           # 주차별 학습 가이드 · 참고자료
```

## 📚 설계 문서

| 문서 | 내용 |
|---|---|
| [`기획서_우주발사.md`](./기획서_우주발사.md) | 게임 기획 전체 (콘셉트·게임플레이·기술 명세·범위) |
| [`주차별_로드맵.md`](./주차별_로드맵.md) | 3주 상세 일정 + 주차별 학습 개념 |
| [`구현_설계_미리보기.md`](./구현_설계_미리보기.md) | 실제 코드 구조 기반 클래스 설계 + 구현 TODO 목록 |
| [`설계_클래스다이어그램.md`](./설계_클래스다이어그램.md) | 클래스 다이어그램 · 소유 관계 · 시퀀스 다이어그램 |
| [`docs/study/`](./docs/study/) | 주차별 학습 가이드 · 실행 가이드라인 · 참고자료 색인 |

## 📄 라이선스

이 저장소의 **소스 코드**는 [MIT License](./LICENSE)를 따릅니다.

`Resources/`(이미지·폰트·사운드)는 학습용으로 포함된 자산이며, 코드와 별도의
라이선스를 따를 수 있습니다.

## 👤 작성자

[@YIDO96](https://github.com/YIDO96)
