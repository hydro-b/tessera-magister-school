# Instructions for AI assistants

You are helping someone make or change a plugin for Tessera, the ESP32 touch screens for Home Assistant. Read this
file first, then the docs in the order below. Everything here is checked by `tools/check.py`; run it before you say a
plugin is done.

## What a plugin is, in five sentences

1. A plugin is a folder: a manifest (`tessera-plugin.yaml`), an ESPHome package (`plugin.yaml`), an ESPHome component
   in C++ (`components/<id>/`), texts (`translations/<language>.json`), a `README.md` and a `CHANGELOG.md`.
2. The screen builds the component into its firmware; the component registers tile types (and cards, top bar items,
   screensavers, ...) with the core through
   `tessera::Plugin` (`components/smart_display/plugin_api.h` in the Tessera repository; a plugin includes it as
   `esphome/components/smart_display/plugin_api.h`).
3. The Tessera app (in Home Assistant) reads only the manifest: it shows the plugin, its options and its README, and
   carries out the plugin's `fetch` (JSON from a web service) with its own code.
4. A tile of a plugin is `plugin:<plugin id>.<tile id>` in a screen's layout. The core gives it a card's drawing area
   while its page is on the glass, hands it the data the app sent (`on_state`), and ticks it once a second (`on_tick`).
5. The plugin API is 0.14 on Tessera's dev branch; the release, Tessera 0.4.88 (firmware 0.56.0), has an older one until
   the next one. A manifest names the lowest minor whose
   parts it uses: `api: "0.8"`. A plugin builds on a core with the
   same major and at least its minor, unless a minor after its own changed a name. So a plugin of 0.7 builds on a core of 0.14.

## Where each kind of thing lives

| The plugin needs | Use | Read |
|---|---|---|
| Data from a web service | A `fetch` the app carries out, mapped to fields | docs/FETCH.md |
| Data Home Assistant already has (a sensor, a calendar, prices in attributes) | A tile of an entity: `domains`, `attributes`, `fields`, `has_attributes` | docs/MANIFEST.md, "Tiles" |
| A setting a person changes | An ESPHome entity in `plugin.yaml`, named in `settings` | docs/MANIFEST.md, "Settings a person changes" |
| Hardware on the board | ESPHome components in `plugin.yaml`, `boards`, optional `parts` | docs/MAKING_A_PLUGIN.md |
| A speaker, microphone or media player for other plugins | `provides`, and the component with the feature's id (`ts_speaker`) in `plugin.yaml` | docs/MAKING_A_PLUGIN.md, "Features" |
| A speaker, microphone or media player another plugin or the board brings | `requires.features`, or a part with `features` when the plugin also works without it | docs/MANIFEST.md, "Features" |
| Another plugin on the same screen | `requires.plugins`: the app adds it with yours | docs/MANIFEST.md, "What it needs and brings" |

## Read in this order

1. [docs/MAKING_A_PLUGIN.md](https://github.com/MaxGramser/tessera-plugins/blob/main/docs/MAKING_A_PLUGIN.md): the files, the steps, trying it on a screen.
2. [docs/MANIFEST.md](https://github.com/MaxGramser/tessera-plugins/blob/main/docs/MANIFEST.md): every field of `tessera-plugin.yaml` and its limits.
3. [docs/FIRMWARE_API.md](https://github.com/MaxGramser/tessera-plugins/blob/main/docs/FIRMWARE_API.md): the C++ API, the life of a tile, the drawing rules.
4. [docs/FETCH.md](https://github.com/MaxGramser/tessera-plugins/blob/main/docs/FETCH.md): data from a web service, the map language, the limits.
5. [docs/PREVIEW.md](https://github.com/MaxGramser/tessera-plugins/blob/main/docs/PREVIEW.md): the bench, to see what the plugin draws on every kind of screen.
6. [docs/TRANSLATIONS.md](https://github.com/MaxGramser/tessera-plugins/blob/main/docs/TRANSLATIONS.md), [docs/TESTING.md](https://github.com/MaxGramser/tessera-plugins/blob/main/docs/TESTING.md),
   [docs/PUBLISHING.md](https://github.com/MaxGramser/tessera-plugins/blob/main/docs/PUBLISHING.md), [docs/LIMITS.md](https://github.com/MaxGramser/tessera-plugins/blob/main/docs/LIMITS.md) when you need them.

## Rules that are never optional

- **Start from the template**: `python3 tools/new_plugin.py <id>`. Do not write the folder from memory. For a
  repository of its own, `python3 tools/new_plugin.py <id> ../<repo>`: it adds an AGENTS.md and CLAUDE.md there that
  point back to this repository's rules and tools.
- **Topics say what it is about**: one or two of the list in `docs/MANIFEST.md`, "What it is about", always. Never a topic of your own; ask for a new one in an issue. What the plugin adds (tiles, a function, hardware)
  is no topic: the app reads it from the manifest.
- **A feature is a promise.** A plugin that `provides` a speaker makes `speaker:` with `id: ts_speaker`; one that needs
  it uses `ts_speaker` and never makes it. Ids that start with `ts_` are Tessera's, for nothing else.
- **plugin.yaml adds, it never takes over**: no `wifi`, `api`, `ota`, `logger`, `esphome`, `http_request`,
  `web_server`, `packages`, `external_components` or the other keys of `docs/LIMITS.md`, "plugin.yaml" (whose one
  exception is ESPHome's update of an ESP32-P4's Wi-Fi co-processor, in a file of its own).
- **The id is the same everywhere, and for ever**: the folder in `plugins/`, `id:` in the manifest, the component
  folder `components/<id>/`, the YAML key in `plugin.yaml` and the C++ namespace `esphome::<id>`. Lowercase letters,
  digits and `_`, starting with a letter, at most 32. Screens, layouts and secrets know the plugin by it: never rename
  it. A fork that keeps the id is another plugin of that id (another origin) and never gets the original's secrets.
- **No words in code or manifest.** Every name, label and hint in the manifest is a key into
  `translations/<language>.json`, part `app`. Every word on the screen is a key of part `screen`, read with
  `plugin->text("key")`. English is complete; other languages may lack keys.
- **Draw like the rest of the screen**: sizes with `tessera::ui::mm()` / `ui::px()`, colours with a theme role
  (`theme::INK`, `theme::MUTED`, `theme::ACCENT`, ...), only the screen's fonts (`tessera::Font`). Never a colour as a
  number, never a font of your own, never an LVGL click handler (use `Tile::on_tap`), never a timer or task (use
  `on_tick`), never a network connection from the screen (data comes through `fetch`, a picture through
  `ui::set_picture`: the app fetches both).
- **Nothing exists off the glass.** Make every LVGL object in `create()` inside `context.parent`. The core deletes them;
  a destructor only frees your own memory.
- **Paint, do not rebuild.** `on_state` and `on_tick` set texts and positions with `ui::set_text`, `ui::set_font`,
  `ui::set_color`, which change a property only when it differs.
- **Name what the plugin may do** in `permissions`: every host a fetch reaches in `network`, every Home Assistant entity
  the screen reads in `read_entities`, every action it calls in `home_assistant_actions`. A plugin with `network` has
  the attribute `cloud` and a `privacy` link.
- **Secrets stay in the app.** An API key is an input of `kind: secret`; it goes into a fetch's header or query, never
  into the URL path, never to the screen, never into YAML. A key for a service Home Assistant talks to (OpenAI,
  Anthropic, Spotify) stays in that Home Assistant integration; the plugin never asks for it.
- **Settings are ESPHome entities.** A value a person changes after the build (a volume, a switch, a word, a test
  button) is a template entity in `plugin.yaml` and a line in the manifest's `settings`, whose `key` is the entity's
  name written as an id ("Tap sound" is `tap_sound`). Never a setting of the plugin's own in the app, never a global
  the editor cannot see.
- **Home Assistant is the backend.** No plugin code runs in the Tessera app. What needs a server goes into Home
  Assistant: an existing integration, or one of your own.
- **Plain English in READMEs**, with a `## Set up` section of numbered steps a person can follow in the Tessera app.
- **A picture for the Plugins page.** Every plugin has a `store.png` beside its manifest, made with
  `tools/store_image.py` once the bench passes, and made again when what the plugin shows changes. It is a thumbnail:
  the name and the `summary` of `translations/en.json` (part `app`) large on the left, the plugin on three screens on
  the right. So keep the summary to one sentence of what a person gets (about 110 characters at most), and put the
  example that shows the plugin best first in `preview.yaml`: the picture lays its pages out with the first examples
  first. Look at the picture before you commit it (docs/PUBLISHING.md, "The picture on the Plugins page").
- **A changelog line for every version.** Raise `version` and add its `## <version>` heading at the top of
  `CHANGELOG.md`, with a few bullet lines of what a person notices and what they must do. The app shows them when it
  offers the update (docs/MAKING_A_PLUGIN.md, "The changelog").

## When you are done

```sh
pip install pyyaml                      # once: the check reads YAML (Homebrew's Python refuses: use the bench's,
                                        # tools/.cache/preview-venv/bin/python tools/check.py, which has it)
python3 tools/check.py plugins/<id>     # must print "<id>: ok"
python3 tools/preview.py plugins/<id> --strict   # the bench: exit 0, no warning in its findings (docs/PREVIEW.md)
python3 tools/store_image.py plugins/<id>        # its picture on the Plugins page: plugins/<id>/store.png
```

**The bench is how you see what you draw.** Run it after every change to the C++, the manifest or `preview.yaml`, not
only at the end, and with `--strict` (exit 2 while a finding is a warning). The first build of a screen takes minutes:
give it a timeout of 20 minutes or run it in the background; `--board cyd` alone is the quick round while the C++
changes. Never pass `--window` or `--watch`, which wait for a person. Read the findings it prints (also in
`.preview/summary.json`), then look at `.preview/overview.png` and the pictures of the smallest and the largest screen,
light and dark (`.preview/<screen>/tile-...png`, `card-...png`): a finding catches what can be measured, a picture what
looks wrong. Give the plugin a `preview.yaml` with data it really gets, so the pictures show it working, not waiting. A
plugin that needs or brings audio or a camera does not build on the bench: say so. docs/PREVIEW.md, "For an
assistant", has the exit codes, the files and the fields of `summary.json`.

**The picture comes last.** When the bench ends without a warning, run `tools/store_image.py`: it writes `store.png`
from the 10-inch, the 4-inch square and the CYD. `--from plugins/<id>/.preview` takes the pictures of the bench run you
just made (its four default screens include those three), so it takes seconds; without it, it benches them again.
Then open `store.png` and look: the name on one line or two, the summary whole (it is cut with an ellipsis only when
three lines of the smallest size do not hold it), and pages that show the plugin with data, not waiting. What it does
with each kind of plugin: tiles fill the three screens; a card shows on the square screen; a plugin with only settings
shows its settings page with its icon; items in the top bar get a loupe over the bar; a plugin of hardware the bench
cannot build (audio, a camera, Bluetooth) gets its icon, large, which is right for it. `check.py` reminds you while
`store.png` is missing, and checks it once it is there (a PNG of 16:9, at most 1 MB). Commit it with the plugin; a
plugin in a repository of its own publishes it with its next release.

The check also fails while the folder still has the template's placeholders: `maintainer: your-github-name`, the
LICENSE's `<year> <your name>`, and the README's line that it is the template.

Then build it on a real screen ([docs/TESTING.md](https://github.com/MaxGramser/tessera-plugins/blob/main/docs/TESTING.md)): push it to GitHub and add the repository's link
in the app (without a release that is a test of the default branch, and a newer push shows as an update), or copy the
folder into Home Assistant's config as `tessera-plugins/<id>/`. A plugin that passes the check but was never on a
screen is not done: say so, and say which board you did not try.
