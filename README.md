# BrückenSpiel

Strategisches Brettspiel »Brücken« — Abschlussprojekt für das Programmierpraktikum (B.Inf.1802) im SoSe 2026.

## Team

| Name | Matrikelnummer | Rolle |
|------|---------------|-------|
|       |               | Projektleiter\*in |
|       |               | |
|       |               | |
|       |               | |

## Spielregeln

Brücken ist ein strategisches Brettspiel für zwei Personen. Ziel ist es, die zwei
gegenüberliegenden Seiten des Spielbretts durch eine durchgehende Brückenverbindung
zu verbinden. Details: [SPIELREGELN.md](https://gitlab.gwdg.de/app/2026ss/bruecken-preset/-/blob/main/SPIELREGELN.md)

## Abhängigkeiten

- C++20-kompatibler Compiler (g++ ≥ 13, clang++ ≥ 18)
- CMake ≥ 3.24
- Raylib 6.0
- Doxygen (optional, für Dokumentation)

## Kompilieren

```bash
cmake -B build
cmake --build build
```

Das ausführbare Programm liegt dann unter `build/bruecken_spiel` (bzw. `build/bruecken_spiel.exe` unter Windows).

## Verwendung

### Mensch gegen Mensch (Standardgröße 24×24)

```bash
./build/bruecken_spiel
```

### Mensch gegen zufällige KI

```bash
./build/bruecken_spiel --playerTypes HUMAN RANDOM_AI
```

### KI gegen KI mit 500 ms Verzögerung

```bash
./build/bruecken_spiel --playerTypes RANDOM_AI RANDOM_AI --delay 500 --gui
```

### Benutzerdefinierte Spielfeldgröße

```bash
./build/bruecken_spiel --size 12 16
```

### Volle Optionen

```bash
./build/bruecken_spiel \
    --size 24 24 \
    --rotation 15 \
    --playerTypes HUMAN SIMPLE_AI \
    --playerNames "Alice" "Bot" \
    --playerColors "#ff0000" "#0000ff" \
    --delay 300 \
    --loglevel INFO
```

## Hilfe

```bash
./build/bruecken_spiel --help
```

Ausgabe:

```
=========================================================================
    BrueckenSpiel <0.1.0>
  Authors: Autor 1, Autor 2, Autor 3, Autor 4
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
 -pc,--playerColors <COLOR ...>The colors of the players. [#RRGGBB ...]
 -pn,--playerNames <NAME ...>  The names of the players.
 -pt,--playerTypes <TYPE ...>  The type of each player. [HUMAN,
                               RANDOM_AI, ...]
_________________________________________________________________________

Preset: v1.0.1 (15.06.2026)
Raylib: 6.0
  This project uses Raylib.
    Website: https://www.raylib.com/
    License: zlib
Implemented optional features:

```

## GUI

Die grafische Oberfläche wird mit Raylib 6.0 realisiert.

- Fenstergröße: 720 × 720 Pixel
- Spielbrett als Raster mit Koordinatenbeschriftung (Zahlen, startend oben links)
- Farbige Markierung der Spielerbereiche (Hex-RGB)
- Linksklick auf das Brett zum Platzieren eines Spielsteins
- Automatische Brückendarstellung zwischen verbundenen Steinen
- Anzeige der gewinnenden Verbindung
- Anzeige des aktuellen Spielers

## Dokumentation generieren

```bash
doxygen Doxyfile
```

Die HTML-Dokumentation liegt dann unter `doc/html/index.html`.
