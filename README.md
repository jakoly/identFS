# identFS

**identFS** ist eine leistungsfähige Datei-Management- und Backup-Lösung für Windows, die Dateien eindeutig identifiziert, Änderungen in Echtzeit erkennt und Projekte strukturiert verwaltet. Pfade dienen nur als Cache – die Wahrheit liegt in UUID und Hash.  

---

## Features

### Eindeutige Dateiidentifikation
- Jede Datei erhält eine **UUID** und einen **Hashwert** (BLAKE3 bevorzugt, SHA-256 optional).  
- Dateien bleiben über Verschiebungen oder Umbenennungen hinweg eindeutig identifizierbar.

### Effiziente Überwachung & Delta-Scan
- **Delta-Scan** prüft nur geänderte oder neue Dateien.  
- **USN Journal** erkennt Änderungen in Echtzeit.  
- Initialer Full-Scan beim Setup erforderlich.  
- Prototyp: `QFileSystemWatcher` für kleinere Volumes.

### Backup & Archivierung
- **Inkrementelle Backups** für maximale Effizienz.  
- **Ultrakomprimierte Snapshots** für Speicheroptimierung.  
- Wiederherstellung über UUID + Hash garantiert Datenintegrität.

### Projektbasierte Organisation
- Logische Gruppierung von Dateien in Projekten.  
- Warnungen beim Löschen von Dateien, die in Projekten referenziert sind.

### Datenbanken
- **File-DB**: uuid, size, hash, last_seen_path, status, last_modified.  
- **Project-DB**: Projekte und Zuordnung der Dateien.  
- **Archive-DB**: Snapshots und Backup-Verwaltung.  
- Optimiert für schnelle Suche und parallele Lese-/Schreibzugriffe.

### UI (Qt)
- Vollbild beim Start, Split-View: Projektliste links, Dateien rechts.  
- Activity-Log für Änderungen, Backups und Warnungen.  
- Buttons für Projekte, Backups und Status jederzeit sichtbar.

### Performance & Threads
- Delta-Scan im Hintergrund.  
- Parallele Hash-Berechnung großer Dateien.  
- SQLite im WAL-Modus für schnelle Lese-/Schreibzugriffe.

### Erweiterungsmöglichkeiten
- Verschlüsselung von Backups.  
- Plugin-System für neue Dateitypen.  
- Plattformübergreifende Version (Linux).  
- Projektübergreifende Duplikat-Erkennung und Versionskontrolle innerhalb von Projekten.

---

## Installation

1. Repository klonen:  
```bash
git clone https://github.com/<username>/identFS.git
