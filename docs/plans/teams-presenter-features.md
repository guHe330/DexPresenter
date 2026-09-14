# Plan: presenter-focused features (share badge, panic key, pacer, teleprompter)

Status: draft, not yet scheduled. Captures a brainstorm about what someone
presenting over Microsoft Teams specifically needs, narrowed to four features
worth building, and how each fits the existing architecture.

Out of scope by design (see the brainstorm that led here): anything needing a
live mic/camera pipeline (mute detector, voice-scroll, gesture control) or a
Teams API integration. DexPresenter's whole pitch is "nothing leaves your
machine, no dependencies" — these four features work by watching the local
screen/OS only, same as everything that exists today.

## Cross-cutting decisions

These affect all four features, so they're worth settling once rather than
per-feature.

### 1. Capture exclusion helper

Three of the four features (badge in its private variant, pacer, teleprompter)
want to be visible to the presenter but invisible in whatever's capturing the
screen (Teams share, OBS, Windows recorder). The primitive is:

```cpp
SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE); // Windows 10 2004+
```

Add a small helper, e.g. `Src/CaptureExclusion.h/.cpp`:

```cpp
namespace CaptureExclusion {
    // Best-effort; returns false on pre-2004 Windows or if the call fails.
    // Callers should still work (just visibly, in the capture) if this fails.
    bool Exclude(HWND hwnd);
}
```

Applied per-window (each overlay that wants it calls this once after
creation). Not a security boundary — legacy GDI BitBlt capture can still see
these windows — so nothing sensitive should ever depend on exclusion actually
working. Document that caveat next to the helper and in the README's Privacy
section, since the honesty-with-limitations tone is already the house style
there (see the "Privacy & Trust" callout in README.md).

### 2. Toolbar real estate

The toolbar is currently a fixed-size strip: `TOOLBAR_NUM_BUTTONS = 5`
(Snip/Draw/Timer/Zoom/Close), width computed directly from that constant
(`Constants.h`), and `Toolbar::m_icons` is a fixed-size array matched to it.
Four new features don't all need a permanent button:

- **Panic key** needs to fire in under a second from a state where the user
  may not even be looking at the toolbar (something private just appeared on
  screen). A toolbar click is too slow for the failure mode it protects
  against — it should be a **global hotkey**, full stop. A toolbar button can
  exist too (for discoverability / mouse users), but the hotkey is the real
  interface. This is a deliberate, documented exception to the "no hotkeys to
  memorize" philosophy — call it out in the README next to the feature.
- **Sharing badge** is a toggle you set up once before a share starts, not a
  per-second action — fits naturally as a **new toolbar button** (6th
  button) or a right-click menu item. Given it's meant to be a habitual
  pre-share check ("did I arm the badge?"), a visible button beats a buried
  menu item.
- **Pacer** and **teleprompter** are session-scoped tools you turn on once at
  the start of a talk and leave running — also reasonable as toolbar buttons,
  but that's 4 new buttons total, doubling the toolbar's width in the worst
  case, which fights the "tiny toolbar" identity.

Recommendation: grow the toolbar's fixed 5-button model into a two-row
concept is overkill for a single dev; instead, ship pacer+teleprompter as a
**single "Presenter" button** that opens a small panel (same right-click
panel pattern Draw already uses) with both toggles in it, rather than two
more top-level icons. Badge gets the 6th slot since it's a single on/off
concept, not a whole panel. Net: toolbar goes from 5 to 6 buttons + Close;
`TOOLBAR_NUM_BUTTONS` and the icon set both need updating, but the panel
pattern for pacer/teleprompter avoids ever needing a 7th or 8th.

### 3. Config schema

Follows the existing per-tool struct pattern in `Config.h` (`ZoomConfig`,
`DrawConfig`, `TimerConfig`, ...). New structs:

```cpp
struct BadgeConfig {
    HotkeyConfig hotkey = { MOD_CONTROL, '4' };
    bool excludeFromCapture = true; // presenter-only by default
    // What's currently armed, so it survives app restarts mid-session:
    // "monitor:\\.\DISPLAY1" or "window" (re-picked on next activation)
};

struct PanicConfig {
    HotkeyConfig hotkey = { MOD_CONTROL | MOD_SHIFT, VK_ESCAPE }; // must not collide with anything
};

struct PacerConfig {
    HotkeyConfig hotkey = { MOD_CONTROL, '5' };
    bool excludeFromCapture = true;
};

struct TeleprompterConfig {
    HotkeyConfig hotkey = { MOD_CONTROL, '7' };
    bool excludeFromCapture = true;
    std::string lastScriptPath; // remember the last-opened script file
};
```

Wire into `AppConfig`, `AppConfigToIni`/`AppConfigFromIni`, and
`SettingsDialog` the same way Zoom/Draw/Timer are wired today. Unit tests
follow the existing INI round-trip test pattern (`tests/`).

### 4. New `WM_ACTIVATE_*` / `HOTKEY_*` IDs

`Constants.h` currently defines `HOTKEY_ZOOM/DRAW/TIMER/SNIP` (values 1/2/3/6
— note 4 and 5 are unused gaps) and matching `WM_ACTIVATE_*` messages. Extend:

```cpp
constexpr int HOTKEY_BADGE        = 4;
constexpr int HOTKEY_PACER        = 5;
constexpr int HOTKEY_TELEPROMPTER = 7;
constexpr int HOTKEY_PANIC        = 8; // global, always registered, not toolbar-driven

constexpr UINT WM_ACTIVATE_BADGE        = WM_USER + 14;
constexpr UINT WM_ACTIVATE_PACER        = WM_USER + 15;
constexpr UINT WM_ACTIVATE_TELEPROMPTER = WM_USER + 16;
constexpr UINT WM_PANIC_TRIGGER         = WM_USER + 17;
```

---

## Feature 1: Sharing badge / border

**Goal:** reduce "wrong window/monitor shared" mistakes by letting the
presenter declare and see what they intend to share.

**UX:**
1. Toolbar button (or right-click menu) opens a lightweight picker: list of
   monitors + a "pick a window" mode (click-to-select, like Snip's capture
   flow already does for a region).
2. On confirm, a colored border (accent pink, per `Theme::Accent()`) is drawn
   around the chosen monitor or window, with a small label
   ("Sharing: DISPLAY1" / "Sharing: PowerPoint Slide Show"). Border must
   track a chosen window if it moves/resizes (poll `GetWindowRect` on a
   timer, same pattern `OverlayWindow::OnTimer` already supports).
3. `excludeFromCapture` (default on) — presenter-only sanity check, doesn't
   itself appear in the Teams feed. Setting it off makes the badge part of
   the visible share (some users may want that for demo/support contexts —
   worth a toggle, not a hidden default).

**Architecture:** new `BadgeTool` overlay window, following `TimerTool`'s
shape (thin subclass of `OverlayWindow`) but non-fullscreen — needs
`OverlayWindow` to support a window sized/positioned to a rect rather than
always covering the whole virtual screen. That's the one base-class change
this feature forces; check whether `Activate()` in `OverlayWindow.cpp`
already parameterizes bounds or hardcodes full-virtual-screen before
committing to this shape.

**Known limitation to document clearly:** there is no API that tells
DexPresenter what Teams actually picked to share. This is a self-declared
confirmation tool, not a Teams-aware sensor. Say so plainly in the UI copy
("Confirm what you're about to share") so nobody mistakes it for automatic
detection.

**Stretch, not in first cut:** auto-arm by watching for Teams' floating
"You're presenting" control-bar window appearing (class-name watch via
`EnumWindows`/`FindWindow`). Unofficial, could silently stop working on a
Teams update — ship manual-arm first, revisit this only if it proves
annoying in practice.

**Effort:** M.

---

## Feature 2: Panic key

**Goal:** one keystroke, blanks every monitor instantly if something private
surfaces mid-share.

**UX:** global hotkey (default `Ctrl+Shift+Esc` — needs to be checked against
Windows' own Task Manager shortcut, which uses exactly that combo; pick a
non-colliding default, e.g. `Ctrl+Shift+B` for "blank," and let it be
rebound in Settings like every other hotkey). Fires instantly:
solid near-black (`Theme::Background()`) full-screen window on **every**
monitor at once (not just the one being shared — the app doesn't reliably
know which one that is, see Feature 1's limitation). Any key or click
dismisses it, same dismiss pattern as `TimerTool`/`ZoomTool`.

**Architecture:** simplest of the four — a `PanicTool` overlay, one instance
per monitor (or one borderless window spanning the full virtual screen,
matching how `CaptureScreen`/`OverlayWindow` already compute virtual-screen
bounds — check `m_screenX/Y/W/H` usage before deciding single-window vs.
per-monitor). No capture-exclusion needed (the whole point is that it *is*
visible in the capture). Must be registered as a **global hotkey via
`RegisterHotKey`** at all times the app is running, not gated behind the
toolbar being visible — it has to work even if the toolbar is hidden.

**Risk:** activation latency. Should be measured — global hotkey → paint
must land well under a second. If GDI+ full-screen paint has noticeable
setup cost (font/brush creation etc. done lazily elsewhere in the codebase),
pre-create the panic window hidden at startup and just `ShowWindow` it on
trigger, rather than constructing it fresh each time.

**Effort:** S. Ship this first — it's the simplest of the four and the one
with the clearest, narrowest failure mode to test.

---

## Feature 3: Pacer

**Goal:** track elapsed time against a planned schedule and show
ahead/behind, distinct from the existing `TimerTool` (which is a public,
audience-visible countdown for breaks).

**UX:** presenter-only (capture-excluded) small widget docked near the
toolbar: elapsed time, and a delta vs. the current expected checkpoint
("+0:40" in red if behind, "-0:15" in green if ahead). Checkpoints come from
a simple schedule — either typed directly (a short list of
"label, target mm:ss" rows in a small dialog) or, once Feature 4 exists,
inherited from the teleprompter script's `[mm:ss]` markers so there's one
source of truth instead of two things to keep in sync.

**Architecture:** new `PacerTool`, structurally close to `TimerTool` (same
`OnTimer`-driven countdown-style tick, same config shape) but counting up
against a schedule instead of down to zero, and capture-excluded by default.
Reuses `CaptureExclusion::Exclude` from the cross-cutting helper.

**Effort:** S–M standalone; trivial once Feature 4's schedule format exists,
since it becomes a reader of that format rather than its own data entry UI.
This is the concrete argument for sequencing pacer after teleprompter (see
Sequencing below), even though pacer alone is simpler.

---

## Feature 4: Teleprompter

**Goal:** presenter-only script/notes view, capture-excluded, manual-advance
by default with optional timestamp-driven pacing.

**How it works (per the brainstorm discussion):** three known models exist
in real teleprompter software — constant-speed auto-scroll (broadcast-style,
needs a human riding the speed dial), manual block-advance (notecards-style),
and timestamp-anchored highlighting (schedule-driven, no forced pace). Voice-
matching auto-scroll (à la PromptSmart) is explicitly excluded — it needs a
live mic pipeline, which is the same privacy-story conflict as the
mute-detector idea from the broader brainstorm.

**Chosen model for v1: manual block-advance, with optional timestamps.**

- **Input format:** plain text/Markdown file the user writes ahead of time.
  Blocks are separated by a blank line. A block may optionally start with a
  `[mm:ss]` marker giving its target time into the talk:

  ```
  [00:00] Welcome — thanks everyone for joining.

  [02:00] Walk through the roadmap slide. Pause for questions.

  Mention the beta signup link before wrapping up.
  ```

  Untagged blocks (like the third one above) just don't participate in pacing
  — they're pure manual-advance content. This means v1 doesn't need any
  timestamp-authoring UI; it's just a text convention, parsed with a small
  regex at load time.

- **Interaction:** current block shown large and centered in a
  capture-excluded panel; previous/next block visible smaller above/below
  for context (classic teleprompter framing). Advance via `Space`/`↓`
  (next), `↑` (previous) while the panel has focus, or via a clicker's extra
  buttons if those already generate keystrokes (most presentation clickers
  do — no new pairing code needed, they just need to be mappable to the same
  keys).

- **Pacer integration:** when a block carries a `[mm:ss]` tag, the
  Pacer (Feature 3) can read the same parsed script and compute
  ahead/behind against elapsed time, instead of requiring a separate
  schedule to be typed twice. This is why the plan sequences teleprompter's
  parser before wiring up Pacer's checkpoint source — build the parser once,
  consume it from both.

**Architecture:**
- `ScriptParser.h/.cpp`: pure parsing (text → `vector<ScriptBlock>{ text,
  optional<Seconds> targetTime }`), no Win32 dependency, so it's unit-testable
  the same way `Config.cpp`'s INI parsing is covered in `tests/`.
- `TeleprompterTool`: `OverlayWindow` subclass, capture-excluded, owns a
  `ScriptParser` result and current-block index, renders via GDI+ text
  layout (check what text measurement/wrapping the codebase already has —
  `SettingsDialog` and the toolbar tooltips are the only text-heavy UI today,
  worth a quick look before writing fresh word-wrap logic).
- File picker for the script: standard `GetOpenFileName` common dialog,
  consistent with how Snip/Draw already save via `SaveUtils`.

**Effort:** M–L. The largest of the four — text layout/wrapping in raw
GDI+ is the main unknown; worth timeboxing a small spike (render one
hardcoded multi-line block with wrapping) before committing to the full
block-navigation UI.

---

## Sequencing recommendation

1. **Panic key** — smallest, most self-contained, highest "why doesn't it
   already do this" value. Also a good forcing function to write the
   capture-exclusion helper's sibling concern (activation latency) early.
2. **Capture-exclusion helper** — build alongside panic key even though
   panic key itself doesn't use it (it's the one feature that *shouldn't* be
   excluded); do it now because #3 and #4 both need it immediately after.
3. **Sharing badge** — validates the "non-fullscreen `OverlayWindow`" base
   class change other features may also want, in a scope small enough to
   absorb that refactor's risk.
4. **Teleprompter** — biggest single piece of new UI; build the script
   parser as an independently testable module first.
5. **Pacer** — cheapest once teleprompter's parser exists; consumes it
   rather than duplicating a schedule-entry UI.

## Open questions (need your call before implementation starts)

- Toolbar layout: is the "6th button + Presenter panel for pacer/teleprompter"
  split above acceptable, or would you rather the toolbar grow a second row?
- Panic key default hotkey: `Ctrl+Shift+B` was a placeholder above — pick
  one that doesn't collide with anything you personally use.
- Badge: default `excludeFromCapture = true` (presenter-only) — confirm
  that's the right default vs. audience-visible.
- Teleprompter script format: plain text with `[mm:ss]` markers, or would
  you rather lean on a structured format (e.g. simple JSON/YAML) for future
  extensibility (per-block notes, speaker cues) at the cost of it being less
  friendly to hand-edit?
