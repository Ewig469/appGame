# Knight Bridge

Knight Bridge ist ein strategisches Brettspiel für zwei Personen und das
Abschlussprojekt für das Programmierpraktikum (B.Inf.1802) im Sommersemester
2026. Ziel ist es, die beiden eigenen gegenüberliegenden Seiten durch eine
durchgehende Folge von Brücken zu verbinden.

## Team

| Name | Rolle und Hauptverantwortung |
|------|------------------------------|
| Hao Guo | Projektleiter; GUI-Fenster, Brett-, Stein- und Brückenrendering sowie Hauptprogramm |
| Zhibo Zhang | Spielfeldmodell und grundlegende Spiellogik |
| Zhixin Fu | Human Player, Random AI Player und Player-Zustandslogik |
| Junke Pu | GUI-Interaktion, Spielstatus und `PlayerGuiAccess`-Integration |

## Voraussetzungen

- CMake 3.25 oder neuer
- C++20-kompatibler Compiler, zum Beispiel GCC 13 oder Clang 18
- Doxygen zur Erzeugung der Projektdokumentation

Preset 1.0.2 und Raylib 6.0 sind im Projekt enthalten und müssen nicht separat
heruntergeladen werden.

## Kompilieren

Im Projektverzeichnis:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Das Programm befindet sich anschließend normalerweise unter
`build/bruecken_spiel`. Bei Mehrkonfigurations-Generatoren, beispielsweise
Visual Studio, kann es unter `build/Release/bruecken_spiel.exe` liegen.

Tests ausführen:

```bash
ctest --test-dir build --output-on-failure
```

## Spiel starten

Mensch gegen Mensch:

```bash
./build/bruecken_spiel
```

Mensch gegen Random AI:

```bash
./build/bruecken_spiel --playerTypes HUMAN RANDOM_AI
```

Random AI gegen Random AI mit sichtbarer GUI und 500 ms Verzögerung:

```bash
./build/bruecken_spiel --playerTypes RANDOM_AI RANDOM_AI --delay 500 --gui
```

Ausführliches Beispiel mit allen implementierten Einstellungen:

```bash
./build/bruecken_spiel \
    --size 24 24 \
    --rotation 15 \
    --playerTypes HUMAN RANDOM_AI \
    --playerNames "Alice" "Bot" \
    --playerColors "#ff0000" "#0000ff" \
    --delay 300 \
    --loglevel INFO
```

Implementiert sind ausschließlich die verpflichtenden Player-Typen `HUMAN`
und `RANDOM_AI`. `SIMPLE_AI` und `ADVANCED_AI` sind nicht implementiert.

## GUI verwenden

- Bei mindestens einem Human Player wird die GUI automatisch geöffnet.
- Ein Spielstein wird mit der linken Maustaste auf einem gültigen Rasterpunkt
  platziert.
- Der obere Statusbereich zeigt den aktuellen Player, die Runde und das
  Spielergebnis.
- Seiten, Spielsteine und Brücken verwenden die übergebenen Player-Farben.
- Der genaue X/Y-Wert jedes Rasterpunkts wird beim Darüberfahren mit der Maus
  angezeigt; dies gilt auch für große Spielfelder bis 96 × 96.
- Nach einem Sieg bleibt das Fenster geöffnet und die Gewinnstrecke wird
  hervorgehoben, bis das Fenster geschlossen wird.
- Bei einer reinen AI-Partie öffnet `--gui` die grafische Darstellung.

## Kommandozeilenhilfe

Aufruf:

```bash
./build/bruecken_spiel --help
```

Ausgabe:

```text
=========================================================================
    Knight Bridge <0.1.0>
  Authors: Zhibo Zhang, Hao Guo, Junke Pu, Zhixin Fu
=========================================================================
Options:
 -d,--delay <DELAY>            Delay in milliseconds after an ai has
                               made its move.
 -g,--gui                      Show the GUI, even if no HUMAN player
                               exists.
 -s,--size <WIDTH HEIGHT>      Dimensions of the board. (5-96)
 -r,--rotation <ROTATION>      Degrees by which to rotate the board.
                               (0.0-90.0)
 -h,--help                     Print this help message and some extra
                               information about this program.
 -ll,--loglevel <LEVEL>        The maximum log level. [ERRORS,
                               WARNINGS, INFO, DEBUG]
 -pc,--playerColors <COLOR ...> The colors of the players. [#RRGGBB ...]
 -pn,--playerNames <NAME ...>  The names of the players.
 -pt,--playerTypes <TYPE ...>  The type of each player. [HUMAN,
                               RANDOM_AI, ...]
_________________________________________________________________________

Preset: v1.0.2 (28.06.2026)
Raylib: 6.0
  This project uses Raylib.
    Website: https://www.raylib.com/
    License: zlib
```

## Dokumentation erzeugen

```bash
doxygen Doxyfile
```

Die HTML-Dokumentation wird unter `doc/html/index.html` erzeugt. Der Ordner
`doc/` ist ein generiertes Artefakt und darf nicht in das GitLab-Repository
committet werden.
