# MazeLab (SFML 3 + CMake + vcpkg, Visual Studio 2026)

Готовый учебный проект: генерация лабиринта + агенты (BFS/A*/partial/manual), визуализация, метрики и лидерборд.

## Фичи (в этом архиве)
- Генерация лабиринта (клеточный, carving по нечётным координатам)
  - DFS (Recursive Backtracker)
  - Randomized Prim
  - Cellular Automata (cave-like, с гарантией пути)
- Start = (1,1), Exit = (W-2,H-2)
- Режимы:
  - Full (агент видит всю карту)
  - Partial (агент получает только SenseWalls4 + TryMove)
- Агенты (один запуск за раз):
  - BFS (Full)
  - A* (Full)
  - Right-hand (Partial)
  - Frontier Explorer (Partial)
  - Manual (ПКМ click-to-move)
- Метрики:
  - steps, path_length, visited_unique, expanded_nodes, replans, duration_ms, status
  - step limit = W*H*20 (после — FAIL)
- Leaderboard:
  - автоматически дописывает `leaderboard.csv` после завершения прохода
  - сохраняет входные параметры (W/H/seed/generator/visibility/agent) + метрики
- SFML GUI:
  - Колёсико — zoom
  - ЛКМ + drag — pan
  - ПКМ по клетке — для Manual: если цель по прямой (ряд/колонка) и нет стен на пути, агент идёт туда по клеткам
  - Панель управления (слева): W/H/Seed/random + генератор/видимость/агент + кнопки + скорость
    - Перетаскивание панели за хедер (ЛКМ)
    - Hide / Show Panel (кнопка Show появляется слева сверху)
    - Detach (F2) — вынести панель в отдельное окно
    - Выбор Generator / Visibility / Agent — **цикличный переключатель** (клик по виджету)
  - После SUCCESS подсвечивается **кратчайший путь** красным

> Важно: размеры W/H приводятся к **нечётным** автоматически (carving-модель требует нечётных размеров, чтобы (W-2,H-2) были проходными узлами).

---

## Сборка в Visual Studio 2026 (CMake project) через vcpkg manifest

### 1) Установи vcpkg (если ещё нет)
```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```

### 2) Открой проект как CMake
- Visual Studio 2026 → **File → Open → Folder...**
- выбери папку `MazeLab`

### 3) Подключи vcpkg к CMake (самый простой вариант)
В Visual Studio:
- **Project → CMake Settings for MazeLab**
- Добавь CMake toolchain:
  - `-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake`

Либо через переменную окружения:
```powershell
setx VCPKG_ROOT C:\vcpkg
```
и добавь в CMake configure:
`-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake`

### 4) Собери и запусти
- Выбери конфигурацию `x64-Debug` или `x64-Release`
- Build → Build All
- Run (зелёная кнопка)

---

## Если vcpkg не найден (ручное подключение SFML)
1) Скачай SFML 3.x под MSVC (или собери).
2) Укажи в CMake:
   - `SFML_DIR` на папку где лежит `SFMLConfig.cmake`
   - или добавь пути include/lib вручную.
3) Всё остальное в проекте уже готово.

---

## Файлы
- `leaderboard.csv` — создаётся рядом с exe после завершения прохода.
