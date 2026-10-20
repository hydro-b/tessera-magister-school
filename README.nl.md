# Magister School Plugin

Een Tessera plugin voor het tonen van schoolinformatie uit Magister via Home Assistant. Toon je rooster, huiswerk en cijfers direct op je Tessera scherm.

## 📋 Vereisten

- **Tessera firmware** met plugin API 0.8 of hoger
- **Magister School Integration** in Home Assistant (https://github.com/OdynBrouwer/magister-school-integration)
- Een Magister sensor entity voor je kind (bijv. `sensor.magister_jan`)

## 🚀 Installatie

1. Voeg deze plugin toe aan je Tessera-systeem via de Plugins-pagina in de app
2. Selecteer de Magister sensor van je kind in de configuratie
3. De tiles verschijnen nu in de layout-editor

## 📱 Beschikbare Tiles

### Volgende Les
Toont de volgende les met vaknaam, lokaal en countdown.

### Rooster
Toont het aantal lessen vandaag.

### Huiswerk
Toont hoeveel huiswerkopenstaand is.

### Cijfers
Toont recente cijfers of gemiddelde.

## ⚙️ Configuratie

In de Tessera app:
1. Ga naar **Plugins** → **Magister School**
2. Kies de sensor voor je kind onder **Child sensor**
3. Voeg tiles toe aan je layout via **Layouts**

## 🎨 Voorbeelden

De plugin biedt verschillende tile-opties:
- **1x1 tile**: Compact overzicht
- **2x1 tile**: Meer details
- **3x2 tile**: Volledig rooster

## 🔧 Technisch

Deze plugin leest data van Home Assistant entities die door de Magister School Integration worden aangemaakt. Geen directe API-synchronisatie - alle data komt via Home Assistant.

## 📝 Privacy

De plugin maakt geen netwerkverbindingen. Alle data wordt gelezen van bestaande Home Assistant entities.

## 🛠️ Ontwikkeling

Deze plugin is gemaakt voor de Tessera-ecosysteem. Bijdragen zijn welkom via issues en pull requests.

## 📄 Licentie

MIT License - zie het LICENSE bestand voor details.

## ⚠️ Disclaimer

Deze plugin is niet officieel geassocieerd met Magister. Gebruik op eigen risico.
