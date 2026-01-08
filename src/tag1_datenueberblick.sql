-- ======================================
-- TAG 1: Datenüberblick verschaffen
-- Datum: 26.12.2024
-- Ziel: Erste Exploration der Wetterdaten
-- ======================================

-- Übung 1: Gesamtanzahl Messungen
-- Ergebnis: 4948 Messungen ✓

SELECT COUNT(*) as GesamtMessungen
FROM dbo.SensorData;



-- ======================================

-- Übung 2: Zeitraum der Daten
SELECT 
      MIN(timestamp) as ErsteMessung,
      MAX(timestamp) as LetzteMessung
      --DATEDIFF(day, MIN(timestamp), MAX(timestamp)) as Anzahltage
FROM dbo.SensorData
WHERE timestamp IS NOT NULL;




-- Zeitraum der Daten (Unix Timestamp konvertieren)
SELECT
    DATEADD(second , MIN(timestamp), '1970-01-01') as ErsteMessung,
    DATEADD(second, MAX(timestamp), '1970-01-01') as LetzteMessung,
    DATEDIFF(day,
        DATEADD(second, MIN(timestamp), '1970-01-01'),
        DATEADD(second, MAX(timestamp), '1970-01-01')
    ) as Anzahltage
FROM dbo.SensorData
WHERE timestamp IS NOT NULL;




SELECT 
    MIN(ReceivedTime) as ErsteDaten,
    MAX(ReceivedTime) as LetzteDaten,
    DATEDIFF(day, MIN(ReceivedTime), MAX(ReceivedTime)) as AnzahlTage
FROM dbo.SensorData;



