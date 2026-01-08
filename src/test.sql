-- Check 1: gibt es Null-Werte im timestamp
SELECT COUNT(*) as NullTimestamp
From dbo.SensorData
WHERE timestamp IS NULL;

---- Check 3: Zeig mir ein paar timestamp-Werte
SELECT TOP 2  timestamp
FROM dbo.SensorData;

-- Check 2: Was ist der Datentyp von timestamp?
SELECT COLUMN_NAME , DATA_TYPE
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_NAME = 'SensorData'
AND COLUMN_NAME = 'timestamp';

--Test für Tag 1
-- Beispiel 1: 7 Tage zu heute addieren
SELECT DATEADD(hour, 7, '2025-12-31 10:00:00') as InEinerWoche;
-- Beispiel 2: Wie viele Tage zwischen zwei Daten?
SELECT DATEDIFF(day, '2024-08-23', GETDATE()) as Anzahltage;

--Beispiel 3: Wie alt ist eine Messung in Minuten ?
SELECT  DATEDIFF(minute, DATEADD(second, timestamp, '1970-01-01'), GETDATE()) as MinutenAlt
FROM dbo.SensorData;

-- MIT Filter - SICHER! ✅
SELECT MIN(timestamp) as minimum, MAX(timestamp) as maximum
FROM dbo.SensorData
WHERE timestamp IS not null


SELECT TOP 5
    Temperature as Temperature
FROM dbo.SensorData;
     
