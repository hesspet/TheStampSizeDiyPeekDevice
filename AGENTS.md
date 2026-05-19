# Übersicht

* Projektname:  The Stamp Size Diy Peek Device

* Developmentenvironment: PlatformIO für ESP32 - Arduino Framework

* Technische Beschreibung der Hardware: ./docs/ESP32-C3 -OLED-Entwicklungsboard.description.md

  * Beschreibung Displayansteuerung
  * Beschreibung Buttons
  * Sample Code
  * PlatformIO Setup-Hinweise, Konfiguration
  * spezifische Hinweise

* Projektverzeichnis soll mehrere PlatformIO Unterprojekte beinhalten

* Jedes Unterprojekt soll eine eigene PROJEKTUEBERSICHT.md Datei beinhalten

* Generische Infos sollen unter ./PROJEKTUEBERSICHT.md abgelegt werden 

* Die Unterprojekte soll in ./PROJEKTUEBERSICHT.md verlinkt werden

* Dokumentation, Codedokumentation auf Deutsch

* Methoden, Variablen etc. sollen Englisch verständliche Namen bekommen

# ESP32 spezifisches

* aktuell wird keine OTA Technik geplant, wenn nicht explizit gefordert

* Konfiguration in PlatformIO soll so angelegt werden, dass möglichst viel Memory für die Programm genutzt werden kann

* Debugausgaben über Serial1 klare Sprache

* Programme sollen alle eine config.h immer beinhalten

* Debugmechanismus in config.h schaltbar none, info, debug, trace

  * Bei none soll aber beim Start trotzdem immer ein kurzer Programmheader mit Compiledatum, Name und ggf. aktueller Konfiguration ausgegeben werden

* Wenn das Projekt mit WIFI Arbeitet, bitte immer ein Konfigurationssystem mit AP vorsehen. Hier sollen ggf. fertige Frameworks genutzt werden.

* Projektbiblotheken sollen unter ./lib heruntergeladen werden und dann auch  ins GIT übernommen werden.

  * Ein Updateverfahren sollte in ./PROJEKTUEBERSICHT.md beschrieben werden

  

