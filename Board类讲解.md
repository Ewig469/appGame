# Board 类讲解

## 文件位置

```
projectX/
├── include/bruecken/
│   ├── board.h      ← Board 类声明
│   └── common.h     ← 基础类型 (Position, Peg, Bridge, Direction, GamePhase)
└── src/
    └── board.cpp    ← Board 类完整实现
```

---

## Board 类概述

`bruecken::Board` 是棋盘游戏的核心类，负责：

| 功能 | 方法 |
|------|------|
| 棋盘创建 | `Board(width, height, rotation)` |
| 坐标/区域判定 | `is_in_bounds()`, `is_corner()`, `get_direction()`, `is_playable()` |
| 走法校验 | `is_valid_move(move)` |
| 执行走法 | `apply_move(move)` |
| 桥梁生成 | `generate_bridges(peg)` (自动调用) |
| 胜负判定 | `check_win(player_id)` |
| 平局判定 | `check_draw()` |

---

## 基本用法

### 创建棋盘

```cpp
#include "bruecken/bruecken.h"

// 默认 24x24，无旋转
bruecken::Board board(24, 24);

// 自定义大小，带旋转
bruecken::Board board2(12, 16, 15.0);   // 12x16, 旋转15°
```

### 走法校验 & 执行

```cpp
// preset::Move(x, y, player_id)
// player_id: 1 = 先手(红, 上/下边), 2 = 后手(蓝, 左/右边)

preset::Move move(3, 5, 1);           // 玩家1要在(3,5)放棋子

if (board.is_valid_move(move)) {      // 三重检查: 越界/区域归属/是否被占
    board.apply_move(move);           // 执行: 放棋子→自动建桥→判定胜负
}
```

### 查询状态

```cpp
board.get_width();       // int     棋盘宽度
board.get_height();      // int     棋盘高度
board.get_phase();       // GamePhase   当前阶段
board.get_turn();        // int     回合数
board.get_current_player(); // int  0=玩家0, 1=玩家1

board.get_pegs();        // std::vector<Peg>    所有棋子
board.get_bridges();     // std::vector<Bridge> 所有桥梁
```

---

## 测试代码

以下测试代码在 `src/main.cpp` 中：

```cpp
#include "argument_parser.h"
#include "logger.h"
#include "move.h"
#include "bruecken/bruecken.h"

int main(int argc, char* argv[]) {
    // 项目元数据
    const std::string project_name = "BrueckenSpiel";
    const std::string project_version = "0.1.0";
    const std::vector<std::string> project_authors = {
        "Autor 1", "Autor 2", "Autor 3", "Autor 4"
    };
    const std::vector<preset::OptionalFeature> implemented_features = {};

    try {
        // 1. 解析命令行参数
        preset::ArgumentParser parser(argc, argv,
                                      project_name, project_version,
                                      project_authors, implemented_features);
        const auto& settings = parser.get_settings();

        // 2. 用解析出的参数创建棋盘
        bruecken::Board board(
            settings.board_config.width,          // 默认 24
            settings.board_config.height,         // 默认 24
            settings.board_config.rotation);      // 默认 0°

        // 3. 测试三个走法
        // 玩家0 (先手) 在 (3,3) 放棋子
        preset::Move m1(3, 3, 1);
        if (board.is_valid_move(m1)) {
            board.apply_move(m1);
        }

        // 玩家1 (后手) 在 (4,5) 放棋子
        preset::Move m2(4, 5, 2);
        if (board.is_valid_move(m2)) {
            board.apply_move(m2);
        }

        // 玩家0 在 (5,4) 放棋子 — 与 (3,3) 是骑士跳关系
        preset::Move m3(5, 4, 1);
        if (board.is_valid_move(m3)) {
            board.apply_move(m3);
        }

        // 4. 打印结果
        preset::Logger::info("Steine: " + std::to_string(board.get_pegs().size()));
        preset::Logger::info("Bruecken: " + std::to_string(board.get_bridges().size()));

    } catch (const std::exception& ex) {
        preset::Logger::error(std::string("Fehler: ") + ex.what());
        return 1;
    }
    return 0;
}
```

---

## 测试结果

```bash
# 编译
cmake -B build
cmake --build build

# 运行
.\build\Debug\bruecken_spiel.exe --loglevel DEBUG
```

### 输出

```
(07:26:16) [INFO]    Starte BrueckenSpiel v0.1.0
(07:26:16) [INFO]    Spielfeld: 24x24, Rotation: 0°
(07:26:16) [INFO]    Board-Grenzen: 5..96
(07:26:16) [INFO]    Board erstellt: 24x24
(07:26:16) [DEBUG]   Stein gesetzt: 3,3 von Spieler 0
(07:26:16) [INFO]    Zug 1 ok: (3,3)
(07:26:16) [DEBUG]   Stein gesetzt: 4,5 von Spieler 1
(07:26:16) [INFO]    Zug 2 ok: (4,5)
(07:26:16) [DEBUG]   Stein gesetzt: 5,4 von Spieler 0
(07:26:16) [DEBUG]   Bruecke gebaut: (5,4) -> (3,3)
(07:26:16) [INFO]    Zug 3 ok: (5,4)
(07:26:16) [INFO]    Steine: 3
(07:26:16) [INFO]    Bruecken: 1
(07:26:16) [INFO]    Spielphase: Laeuft
```

### 逐行解读

| 输出 | 含义解释 |
|------|----------|
| `Starte BrueckenSpiel v0.1.0` | 程序启动，版本号 |
| `Spielfeld: 24x24, Rotation: 0°` | 棋盘 24×24，无旋转 |
| `Board-Grenzen: 5..96` | 棋盘尺寸范围 5-96 |
| `Board erstellt: 24x24` | Board 对象构造成功 |
| `Stein gesetzt: 3,3 von Spieler 0` | 玩家0（先手/红）在坐标(3,3)落子 |
| `Zug 1 ok: (3,3)` | 第一次走法通过校验 `is_valid_move()` 返回 `true` |
| `Stein gesetzt: 4,5 von Spieler 1` | 玩家1（后手/蓝）在坐标(4,5)落子 |
| `Zug 2 ok: (4,5)` | 第二次走法通过校验 |
| `Stein gesetzt: 5,4 von Spieler 0` | 玩家0 在坐标(5,4)落子 |
| `Zug 3 ok: (5,4)` | 第三次走法通过校验 |
| `Bruecke gebaut: (5,4) -> (3,3)` | **关键！** 检测到 (5,4) 和 (3,3) 是骑士跳关系<br>dx=2, dy=1 → 自动生成桥梁 |
| `Steine: 3` | 棋盘上共 3 个棋子 |
| `Bruecken: 1` | 自动生成了 1 座桥梁 |
| `Spielphase: Laeuft` | 游戏仍在进行中（InProgress） |

---

## 关键算法说明

### 骑士跳 (Rösselsprung)

```
  · · × · × · ·      ○ = 新棋子 (5,4)
  · × · · · × ·      × = 8个可能桥接点 
  · · · ○ · · ·      已有棋子 (3,3) 在 × 之一 → 建桥
  · × · · · × ·
  · · × · × · ·      条件: dx=1,dy=2 或 dx=2,dy=1
```

代码中使用了 8 个偏移量：
```cpp
{ 1, -2}, { 2, -1}, { 2,  1}, { 1,  2},
{-1,  2}, {-2,  1}, {-2, -1}, {-1, -2}
```

### 桥梁交叉检测

用叉积方向法判断两条线段是否相交（不含端点接触）：

```
A ─────────── B      桥 AB 和 CD:
    ╲    ╱
     ╲  ╱            orient(A,B,C) ≠ orient(A,B,D)
      ╳              orient(C,D,A) ≠ orient(C,D,B)
     ╱  ╲            → 相交 ❌
    ╱    ╲
C ─────────── D
```

### 胜负判定 (BFS)

```
玩家0: 从 上边(Top) 出发 → BFS沿桥梁走 → 能否到达 下边(Bottom)?
玩家1: 从 左边(Left) 出发 → BFS沿桥梁走 → 能否到达 右边(Right)?
```

---

## 区域划分

```
┌───┬──────────────┬───┐
│ X │  上边(玩家0)  │ X │   X = 四角（重叠区，双方都不能放）
├───┼──────────────┼───┤
│   │              │   │
│左 │  内部区域     │右 │   内部 = 双方都能放棋子
│边 │  (N-2)×(M-2) │边 │
│(1)│              │(1)│
│   │              │   │
├───┼──────────────┼───┤
│ X │  下边(玩家0)  │ X │
└───┴──────────────┴───┘
```

- 玩家0（先手）能在：内部 + 上边 + 下边 放棋子
- 玩家1（后手）能在：内部 + 左边 + 右边 放棋子
- 四个角：双方都不能放

---

## 接口速查表

| 方法 | 参数 | 返回 | 说明 |
|------|------|------|------|
| `Board(w, h, r)` | 宽, 高, 旋转角 | — | 构造函数，校验5-96、0-90° |
| `get_width()` | — | int | 棋盘宽度 |
| `get_height()` | — | int | 棋盘高度 |
| `get_phase()` | — | GamePhase | 游戏阶段枚举 |
| `get_turn()` | — | int | 当前回合数 |
| `get_current_player()` | — | int | 当前玩家(0或1) |
| `get_pegs()` | — | `vector<Peg>&` | 所有棋子列表 |
| `get_bridges()` | — | `vector<Bridge>&` | 所有桥梁列表 |
| `is_in_bounds(pos)` | Position | bool | 是否在棋盘内 |
| `get_direction(pos)` | Position | Direction | 区域归属 |
| `is_playable(pos, pid)` | Position, int | bool | 某玩家能否在此落子 |
| `is_occupied(pos)` | Position | bool | 该位置是否有棋子 |
| `is_valid_move(move)` | preset::Move | bool | 走法是否合法 |
| `apply_move(move)` | preset::Move | void | 执行走法 |
| `check_win(pid)` | int | bool | 某玩家是否获胜 |
| `check_draw()` | — | bool | 是否平局 |
