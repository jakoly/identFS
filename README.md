# IdentFS

**IdentFS** ist ein Windows-Desktop-System (C++ / Qt), das klassische Dateisysteme erweitert, ohne sie zu ersetzen.  
Im Mittelpunkt steht **Identität statt Pfad**: Dateien werden über **UUIDs und Metadaten** verwaltet und bleiben auch dann auffindbar, wenn sie außerhalb der Anwendung verschoben, umbenannt oder reorganisiert werden.

IdentFS ist kein Explorer-Klon, sondern eine **autarke, selbstheilende Dateiverwaltungsschicht** mit Projektlogik, ultrastarker Kompression und intelligenter Sicherung.

---

## 🔥 Kernidee

> **Pfade sind instabil – Identitäten nicht.**

Klassische Dateiverwaltung bricht, sobald:
- Dateien verschoben werden
- Ordnerstrukturen geändert werden
- Backups zurückgespielt werden

IdentFS löst dieses Problem durch:
- eindeutige Datei-Identitäten (UUID)
- Metadaten-basierte Wiedererkennung
- getrennte, spezialisierte Datenbanken
- kontinuierliche Konsistenzprüfung im Hintergrund

---

## 🧠 Grundprinzipien

- **Keine Pfade als Wahrheit**
- **Autarker Betrieb** (Explorer-Aktionen werden erkannt, nicht verboten)
- **Trennung von Daten, Beziehungen und Backups**
- **Kein Vendor-Lock-in**
- **Alles bleibt normale Dateien auf NTFS**

---

## 🧩 Systemarchitektur

### 1️⃣ File-DB (Identitätsdatenbank)
Speichert **jede relevante Datei** mit:

- UUID (Primärschlüssel)
- Dateigröße
- Hash (z. B. BLAKE3 / SHA-256)
- Zeitstempel
- optionale NTFS-Metadaten
- interner Status (aktiv, verschoben, vermisst)

➡ Erkennt Dateien **wieder**, selbst wenn sie:
- verschoben
- umbenannt
- in andere Ordner kopiert werden

---

### 2️⃣ Project-DB (Beziehungsdatenbank)

- Projekte mit eigener UUID
- Viele-zu-Viele-Relation:
  - Projekt ↔ Dateien
- Projektmetadaten:
  - Beschreibung
  - Tags
  - Versionen

➡ Eine Datei kann **in mehreren Projekten gleichzeitig existieren**, ohne dupliziert zu werden.

---

### 3️⃣ Backup- & Archive-DB

- Ultrakomprimierte Sicherungen
- Versionierte Projekt-Snapshots
- Inkrementelle Archivstruktur
- Wiederherstellung **unabhängig vom ursprünglichen Pfad**

---

## 🗂 Dateiidentifikation (UUID)

### Wo wird die UUID gespeichert?

**Mehrschichtiges System (Fail-Safe):**

1. **NTFS Alternate Data Stream (ADS)** *(primär)*
2. **Interne Datenbank**
3. **Hash-basierte Wiedererkennung** *(Fallback)*

➡ Kein vollständiger Festplatten-Scan nötig  
➡ Keine Abhängigkeit von Dateipfaden  
➡ Explorer-Aktionen bleiben erlaubt

---

## 🧨 Löschschutz & Warnsystem

Wenn eine Datei:
- Teil eines Projekts ist
- in einer Relation steht
- in einem Archiv referenziert wird

➡ **Warnung vor dem Löschen**
➡ kein Zwang, aber **informierte Entscheidung**

---

## 🧱 Kompression & Archivierung

- Eigener Container (kein ZIP-Wrapper)
- Fokus:
  - viele kleine Dateien
  - redundante Daten
  - Medien-Workflows
- Ziel:
  - **deutlich besser als ZIP**
  - konkurrenzfähig zu 7z, aber projektbewusst
