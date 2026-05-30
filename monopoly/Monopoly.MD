Monopoly

1. Spieler
- Spieler Anzahl wählen
- Spieler initieren mit Startwerten (Name,Geld,Startfeld usw.)
- Spielerreihenfolge entscheiden
- Spieler bewegt sich auf dem Brett durch Würfeln

2. Spielbrett

-- Spielbrett besteht aus Feldern
-- Die Felder:
--Straßen (Sonderstraßen ggf. Bahnhöfe/Stationen, Elektrizität/Wasserwerk)
--Gehe über Los
--Gehe ins Gefängnis
--Frei Parken

- Straßen:
-- Name
-- Kaufpreis
-- Farbe
-- Farbgruppe
-- Miete
-- Häuser bauen

- Gehe über los:
-- Man erhält Geld wenn man das Feld passiert

- Gehe ins Gefängnis:
-- Wenn man auf dem Feld landet, geht man zum Gefängnisfeld
-- Gefängnis: Frei Spielen, Frei Kaufen, sonst 3 Runden aussetzen

- Frei Parken
-- nichts machen (ggf. Minispiel)

3. Visualisierung

-- Spielbrett im Terminal
-- Brett als einzelne Boxen der Felder
-- "Kamera" bewegt sich zum jetzigen Spieler

4. Ende
-- Programm endet falls alle Spieler außer einer keine Schulden hat.

## Single field design

┌ ┐ └ ┘ ├ ┤ ┬ ┴ ┼
─
│



Minimap:
┼───┼
│  │
┼───┼

Player:
󱕫

Dice:
char * dice_one = "      "
"     "
"      ";

"     "
"      "
"     "

"     "
"     "
"     "

"    "
"      "
"    "

"    "
"     "
"    "

"    "
"    "
"    "
