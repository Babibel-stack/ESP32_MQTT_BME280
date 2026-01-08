--Aufgabe 1: Wie viele Messungen sind insgesamt in der Datenbank gespeichert?
SELECT COUNT(*) as Gesamtwerte
FROM dbo.TelemetryData


--10 letzte Temperaturwerte der letzten 24 Stunden
SELECT TOP 10
    Timestamp,
    Temperature
FROM dbo.TelemetryData
WHERE DATEADD(second,Timestamp, '1970-1-1') > DATEADD(hour, -24, GETDATE())
    AND Temperature IS NOT NULL
ORDER BY Timestamp  DESC


--Aufgabe 2: Was ist die höchste gemessene Temperatur?
SELECT 
    MAX(Temperature) as groessteTemperature
FROM dbo.TelemetryData


--Aufgabe 3: Was ist die niedrigste Luftfeuchtigkeit?
SELECT 
    MIN(Humidity) as niedrigsteHumidity
FROM dbo.TelemetryData


--Aufgabe 4: Zeige die erste und letzte Messung (älteste und neueste) an.
    --Die Erste Messung 
SELECT Top 1
    Timestamp,
    Temperature,
    Humidity,
    Pressure  
FROM dbo.TelemetryData
ORDER BY Timestamp ASC

    --Die letzte Messung
SELECT Top 1
    Timestamp,
    Temperature,
    Humidity,
    Pressure  
FROM dbo.TelemetryData
ORDER BY Timestamp DESC    


--Kürzere Version der Aufgabe 4
SELECT
    MIN(DATEADD(second,Timestamp, '1970-1-1')) as ersteMessung,
    MAX(DATEADD(second,Timestamp, '1970-1-1')) as letzteMessung
FROM dbo.TelemetryData

--Aufgabe 5: Zähle, wie viele verschiedene Geräte (DeviceId) Daten gesendet haben.
SELECT COUNT(DISTINCT  DeviceId) as anzahlDevice
FROM dbo.TelemetryData

--Alle Werte vor 7 Tagen 
SELECT * 
    FROM dbo.TelemetryData
WHERE DATEADD(second, Timestamp, '1970-1-1') >= DATEADD(day, -7, GETDATE())
ORDER BY timestamp ASC


--📈 Mittlere Aufgaben (Aggregationen)
--Aufgabe 6: Berechne die durchschnittliche Temperatur, Luftfeuchtigkeit und Luftdruck aller Messungen.
SELECT 
    AVG(Temperature) as TempAverage,
    AVG(Humidity)as HumAverage,
    AVG(Pressure) as PressAverage
FROM dbo.TelemetryData


--Aufgabe 7: Finde die Temperatur-Spannweite (Differenz zwischen höchstem und niedrigstem Wert).

SELECT
     MAX(Temperature) - MIN(Temperature)  as TemperaturSpannweite
FROM dbo.TelemetryData

--Aufgabe 8: Wie viele Messungen wurden in den letzten 7 Tagen gespeichert?
SELECT COUNT(*) as anzahlMessungvor7Tage
FROM dbo.TelemetryData
WHERE DATEADD(second, Timestamp, '1970-1-1') >= DATEADD(day, -7, GETDATE())
ORDER BY Timestamp DESC 

--Aufgabe 9: Zeige pro Tag die Anzahl der Messungen .

SELECT 
    CAST(DATEADD(second, Timestamp, '1970-1-1') AS DATE) as Datum,
    COUNT(*) as AnzahlMessungen
FROM dbo.TelemetryData
GROUP BY CAST(DATEADD(second, Timestamp, '1970-1-1') AS DATE)
ORDER BY Datum DESC

--Aufgabe 9: Zeige pro Stunde die Anzahl der Messungen .
--Aufgabe 9: Zeige pro Stunde die Anzahl der Messungen
SELECT 
    DATEADD(hour, DATEDIFF(hour, 0, DATEADD(second, Timestamp, '1970-1-1')), 0) as Stunde,
    COUNT(*) as AnzahlMessungProStunde
FROM dbo.TelemetryData
GROUP BY DATEADD(hour, DATEDIFF(hour, 0, DATEADD(second, Timestamp, '1970-1-1')), 0)
ORDER BY Stunde DESC


--Aufgabe 10: Finde alle Messungen, wo die Temperatur über 25°C liegt.
SELECT 
    Timestamp,
   Temperature
FROM  dbo.TelemetryData
WHERE Temperature >= 25
ORDER BY Timestamp DESC


--Aufgabe 10: Anzahl der Messungen, wo die Temperatur über 25°C liegt.
SELECT COUNT(*) as AnzahlTemperatureueber25Grad
FROM  dbo.TelemetryData
WHERE Temperature >= 25


--🔍 Fortgeschrittene Aufgaben (Analyse)
--Aufgabe 11: Berechne die durchschnittliche Temperatur pro Stunde der letzten 24 Stunden.
SELECT 
    FORMAT(DATEADD(second, Timestamp, '1970-1-1'), 'yyyy-MM-dd HH:00') as Stunde,
    AVG(Temperature) as DurchschnittTemperatur,
    COUNT(*) as AnzahlMessungen
FROM dbo.TelemetryData
WHERE DATEADD(second, Timestamp, '1970-1-1') >= DATEADD(hour, -24, GETDATE())
GROUP BY FORMAT(DATEADD(second, Timestamp, '1970-1-1'), 'yyyy-MM-dd HH:00')
ORDER BY Stunde DESC



--Aufgabe 12: Finde die "heißeste" und "kälteste" Stunde des Tages (Durchschnitt pro Stunde).
-- Heißeste Stunde
SELECT TOP 1
    FORMAT(DATEADD(second, Timestamp, '1970-1-1'), 'HH:00') as Stunde,
    AVG(Temperature) as DurchschnittTemperatur
FROM dbo.TelemetryData
GROUP BY FORMAT(DATEADD(second, Timestamp, '1970-1-1'), 'HH:00')
ORDER BY AVG(Temperature) DESC

-- Kälteste Stunde
SELECT TOP 1
    FORMAT(DATEADD(second, Timestamp, '1970-1-1'), 'HH:00') as Stunde,
    AVG(Temperature) as DurchschnittTemperatur
FROM dbo.TelemetryData
GROUP BY FORMAT(DATEADD(second, Timestamp, '1970-1-1'), 'HH:00')
ORDER BY AVG(Temperature) ASC


--Aufgabe 13: Zeige alle Messungen, wo gleichzeitig Temperatur > 20°C UND Luftfeuchtigkeit < 50% ist.
SELECT 
    DATEADD(second, Timestamp, '1970-1-1') as Zeitpunkt,
    Temperature,
    Humidity,
    Pressure
FROM dbo.TelemetryData
WHERE Temperature > 20 
    AND Humidity < 50
ORDER BY Timestamp DESC



--Aufgabe 14: Erstelle eine Kategorisierung der Temperatur:

--Kalt: < 15°C
--Angenehm: 15-25°C
--Warm: > 25°C

SELECT 
    DATEADD(second, Timestamp, '1970-1-1') as Zeitpunkt,
    Temperature,
    CASE 
        WHEN Temperature < 15 THEN 'Kalt'
        WHEN Temperature >= 15 AND Temperature <= 25 THEN 'Angenehm'
        WHEN Temperature > 25 THEN 'Warm'
        ELSE 'Unbekannt'
    END as Kategorie
FROM dbo.TelemetryData
WHERE Temperature IS NOT NULL
ORDER BY Timestamp DESC


--Aufgabe 15: Finde Messungen mit "extremen" Werten (z.B. Temperatur außerhalb von 2 Standardabweichungen vom Durchschnitt).
SELECT 
    CASE 
        WHEN Temperature < 15 THEN 'Kalt'
        WHEN Temperature >= 15 AND Temperature <= 25 THEN 'Angenehm'
        WHEN Temperature > 25 THEN 'Warm'
    END as Kategorie,
    COUNT(*) as Anzahl,
    ROUND(AVG(Temperature), 2) as DurchschnittTemperatur
FROM dbo.TelemetryData
WHERE Temperature IS NOT NULL
GROUP BY 
    CASE 
        WHEN Temperature < 15 THEN 'Kalt'
        WHEN Temperature >= 15 AND Temperature <= 25 THEN 'Angenehm'
        WHEN Temperature > 25 THEN 'Warm'
    END
ORDER BY Kategorie