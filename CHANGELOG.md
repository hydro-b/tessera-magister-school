# Changelog

## 0.1.8 - 2026-10-20
- Add verbose debug logging to diagnose data issues
- Log all keys and values to see what data is received
- Will help identify why data is not showing

## 0.1.7 - 2026-10-20
- Fix: Read attributes from correct location in entity data
- Data structure is {state, name, attributes: {...}} not flat
- Better logging to show state and attribute values

## 0.1.6 - 2026-10-20
- Fix C++ syntax errors in ScheduleTile
- Fallback: count afspraken array for today if attribute is missing
- Handle both 'aantal_afspraken_vandaag' and 'Aantal afspraken vandaag'

## 0.1.5 - 2026-10-20
- Fix C++ compilation errors (ArduinoJson API, time_t types)

## 0.1.4 - 2026-10-20
- Fix: Read correct Magister attributes
- Parse next lesson from afspraken array

## 0.1.1 - 2026-10-20
- Fix: Read correct Magister attributes (afspraken array, aantal_afspraken_vandaag)
- Parse next lesson from afspraken (vak, lokaal, start time)
- Fix C++ compilation errors (ArduinoJson API, time_t types)

## 0.1.0 - 2026-10-20
- Initial beta release with Next lesson, Schedule, Homework, and Grades tiles
- Support for multiple tile sizes (1x1, 2x1, 3x2)
- Dutch and English translations
- Tap actions to open full cards (placeholder)
- Beta version, still under development
