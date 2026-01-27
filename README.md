# identFS — Die smarte Ergänzung für den Windows Explorer 🗂️✨

**identFS** ist kein Ersatz für den Explorer. Es ist ein leichtgewichtiger, massentauglicher Ergänzungsdienst, der genau da eingreift, wo der Explorer an Grenzen stößt: zuverlässige Identifikation, sichere Backups, projektbasierte Organisation und einfache Wiederherstellung — alles ohne den gewohnten Workflow zu ändern. 🚀


## Warum identFS zusätzlich zum Explorer sinnvoll ist 💡
- **Explorer = Oberfläche. identFS = Wahrheit.** Pfade sind nur Cache; identFS speichert **UUID + Hash**. Verschieben, Umbenennen oder Netzlaufwerk-Wechsel? Kein Problem! 🔄  
- **Schnelleres Finden & Wiederherstellen.** Dateien, Duplikate oder Projektzugehörigkeiten aufspüren — unabhängig vom Pfad. 🔍  
- **Sichere Backups ohne Aufwand.** Inkrementelle, ultrakomprimierte Snapshots. 💾  
- **Keine neue Denkweise nötig.** Explorer bleibt erhalten, identFS ergänzt über Kontextmenü, UI-Buttons und Dashboard. 🖱️


## Zielgruppe / Massentauglichkeit 🎯
identFS eignet sich für:  
- Privatanwender, die Ordnung, Backups und einfache Wiederherstellung wollen ✅  
- Kreative (Fotos, Videos, Projekte), die Dateien logisch bündeln möchten 🎨  
- Kleine Teams, die Versionen & Referenzen zuverlässig verwalten möchten 👥  

**Massentauglich =** Ein-Klick-Installation, sinnvolle Standard-Einstellungen, geringe Systemlast und intuitive UI. ⚡


## Kerneigenschaften (für Anwender verständlich) 🛠️
- **Nahtlose Integration:** Kontextmenü im Explorer, Drag & Drop, Thumbnail-Preview 🖼️  
- **Automatische Überwachung:** Windows USN-Journal, Delta-Scans nach initialem Full-Scan ⏱️  
- **Projektansicht:** Dateien Projekten zuordnen, Warnungen beim Löschen 🔔  
- **Sichere Backups:** Inkrementelle Snapshots, hohe Kompression, einfache Wiederherstellung 💾  
- **Duplikat- und Versionshinweise:** Erkennen & Zusammenführen 🔗  
- **Privatsphäre & Sicherheit:** Lokale Speicherung, optionale Verschlüsselung 🔒  
- **Ressourcen-schonend:** Hintergrundprozesse mit niedriger Priorität 🖥️


## Vorteile gegenüber alleiniger Nutzung des Explorers 🆚
- identFS sichert **Identität**, nicht nur Pfade.  
- Projektübergreifende Duplikaterkennung und ultrakomprimierte Snapshots.  
- Einfache, sichere Wiederherstellung auch bei Umbenennungen oder Verschiebungen. 🔄


## Quick Start (in 3 Schritten) ⚡
1. Installer ausführen (Windows 10/11). 🖱️  
2. Initial-Scan starten (je nach Datenmenge etwas Zeit ⏳).  
3. Explorer öffnen: neue Kontextoptionen & Dashboard sind verfügbar ✅


## Technische Kurzinfos 🖥️
- Hash-Standard: **BLAKE3** (schnell), optional SHA-256  
- Überwachung: **USN Journal** (produktiv), `QFileSystemWatcher` (Prototyp/Test)  
- Datenbanken: `file.db`, `project.db`, `archive.db` (SQLite, WAL-Modus)  
- UI: Qt (Split-View, Activity-Log, Statusleiste)  
- Systemlast: konfigurierbare Threadpools für Hashing & Delta-Scanner ⚙️


## Bedienkomfort & Support 🆘
- Vorkonfigurierte Profile: „Privat“, „Kreativ“, „Power-User“ ⚡  
- One-click Backup / Restore 🔄  
- Hilfeseiten & FAQ im Installer 📖  
- Optionaler Telemetrie-Opt-in zur Fehleranalyse 📊


## Lizenz & Mitmachen 📝
- Open-Source, **MIT License**. Beiträge willkommen — bitte Issues oder Pull Requests eröffnen. 🤝


## Fazit ✅
identFS ergänzt den Windows Explorer dort, wo zuverlässige Identität, sichere Backups und projektorientierte Organisation gebraucht werden — **ohne den gewohnten Workflow zu ersetzen**.  
Für Anwender bedeutet das: mehr Sicherheit 🔒, bessere Auffindbarkeit 🔍 und einfache Wiederherstellung 🔄 bei minimalem Aufwand.
