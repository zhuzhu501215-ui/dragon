# Team layout (headers co-located with owners)

There is **no** central `include/` tree. Each teammate keeps **both** `.h` and `.cpp` in `src/<your-name>/`. The Makefile adds `-Isrc/<name>` for every folder so `#include "game_data.h"` still works when that file lives in `src/wu-yiming/`.

| Teammate | Folder | Headers | Sources | Role |
|----------|--------|---------|---------|------|
| **Scott** | `src/scott/` | `utils.h`, `console_ui.h` | `utils.cpp`, `console_ui.cpp`, `main.cpp` | RNG / console helpers, shared panels (`printHeader`, hero panel), **root Makefile**, program entry |
| **Wu-Yiming** | `src/wu-yiming/` | `game_data.h`, `shop_system.h` | `game_data.cpp`, `shop_system.cpp` | Core structs, equipment ctor, **shop catalog + shop loop** (economy) |
| **Enya** | `src/enya/` | `monster_system.h`, `hell_mode.h` | `monster_system.cpp`, `hell_mode.cpp` | Encounters, scaling, drops, boss multiplier prompt, **Hell Mode + letter grades** |
| **Francis** | `src/francis/` | `hero_system.h`, `battle_view.h` | `hero_system.cpp`, `battle_view.cpp` | Heroes, ASCII art, **battle screen / monster sprites** |
| **Victor** | `src/victor/` | `battle_system.h` | `battle_system.cpp` | **Combat rules only** (turn loop, damage, `runBattle`) — no Hell UI, no ASCII field |
| **Zhang-Peihan** | `src/zhang-peihan/` | `ui_flow.h` | `ui_flow.cpp` | Tutorials, stage wrapper, title / game-over screens, `runStage` orchestration |

`shop_system.cpp` calls `printTutorialShopHint` implemented in `ui_flow.cpp`; a forward declaration in the shop TU avoids a header cycle.

## Build

```bash
make          # → build/gp
make rebuild
```

## Workflow

- Touching someone else’s `.h` may force rebases — coordinate in the PR.
- Run `make` before pushing to `main`.

## CODEOWNERS

Update `.github/CODEOWNERS.example` with real GitHub handles and copy to `CODEOWNERS`.
