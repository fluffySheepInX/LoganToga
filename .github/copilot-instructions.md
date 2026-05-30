# Copilot Instructions

## Project Guidelines
- For LoganToga2Remake2 configuration/data files, prefer TOML instead of JSON.
- For LoganToga2Remake2 localization changes, prefer a TOML-based approach for language resources.
- LoganToga2Remake2 の兵士デザインは『弱いがストレスではなく、過度な操作量を要求しない』方向を優先する。
- For this codebase, use plain Rect-based custom UI for visible menu controls instead of SimpleGUI when button visibility is an issue。
- For volume settings UI in LoganToga2Remake2, use WindowChromeAddon.
- When adding new units in this codebase, ask the user whether the unit should be included in the initial unlock set.
- For LoganToga2Remake2, persistent settings files should follow the existing Local/AppData save-location switch and be stored alongside the continue-run save data.
- Add comments above method names in this codebase as per user preference.
- For complex changes, follow the principle of "Think and Plan as a Mathematician," meaning to think rigorously and plan before proceeding, as the user prefers a logical and precise thinking style.
- For allfunc, design the target selection not fixed to enemies, allowing for future Heal skills to be expanded to allies。

## Project SKY Guidelines
- When the user says Project SKY, work on the SKY project/files rather than LoganToga2Remake2 unless they specify otherwise。

## LoganToga3 Guidelines
- For LoganToga3 editor UI, use Rect-based buttons and do not use SimpleGUI; future editor interfaces should be added to the editor toolbar/bar。
- In LoganToga3, identify decals by the filename prefix `decal_`. When right-clicking a non-decal asset, add the `decal_` prefix to decal-ize it。
- For LoganToga3 TOML writes, do not use any API or pattern that automatically appends newline codes per line or at the end during output; build the full text first and add only the minimum required newlines explicitly。
- For LoganToga3, unify asset specifications such as SkillIcon image sizes to be multiples of 32。
- For LoganToga3 SkillIcon assets, require canvas width/height to be multiples of 32, require the main image and frame image layers to use the same canvas size, and treat icon alignment as center-origin based。
- LoganToga3 の SkillIcon 命名規約は `w_xx_name` を枠画像、`n_name` を主画像とし、`w` は最後に自動整列、`xx` は最大2桁のソート順として扱う。
- LoganToga3 の Editor 数値調整 UI は、共通化・直接入力・step 値表示/変更・Shift/Ctrl による一時的な粗/細調整・右クリックメニュー補助を基本方針とする。
- For LoganToga3, design the music editor as a foundational music management system for all scenes, including title, combat, and future options or achievement screens, rather than being limited to specific screens。
