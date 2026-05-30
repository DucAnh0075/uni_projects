# Monopoly

Ein kleines Monopoly-Projekt mit grafischer Darstellung und Spiellogik in C mit nur C-Standard Librarys.

## Voraussetzungen

- `gcc`
- `make`
- eine Umgebung, in der das Projektfenster gestartet werden kann

## Bauen

Im Projektverzeichnis einfach ausführen:

```bash
make
```

Dadurch wird das Programm als `solution` gebaut.

## Starten

Nach dem Build kannst du das Spiel mit folgendem Befehl starten:

```bash
./solution
```

Optionale Flags:

- `--debug` aktiviert den Debug-Modus
- `--log` aktiviert zusätzliches Logging

Beispiel:

```bash
./solution --debug --log
```

## Wichtiger Hinweis zur Fenstergröße

Das Spiel darf nur in einem eher kleinen Fenster gestartet werden. Wenn das Fenster zu groß ist, kann es zu einem Segmentation Fault kommen.

Wenn du das Projekt ausführst, verwende deshalb keine zu große Fenstergröße.

## Aufräumen

```bash
make clean
```

Damit werden das Binary und die Objektdateien entfernt.