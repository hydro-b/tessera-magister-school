# Magister School Plugin

A Tessera plugin for displaying school information from Magister via Home Assistant. Show your schedule, homework, and grades directly on your Tessera screen.

## 📋 Requirements

- **Tessera firmware** with plugin API 0.8 or higher
- **Magister School Integration** in Home Assistant (https://github.com/OdynBrouwer/magister-school-integration)
- A Magister sensor entity for your child (e.g., `sensor.magister_jan`)

## 🚀 Installation

1. Add this plugin to your Tessera system via the Plugins page in the app
2. Select your child's Magister sensor in the configuration
3. The tiles will now appear in the layout editor

## Set up

To get the plugin working:

1. **Install the Magister School Integration** in Home Assistant first (https://github.com/OdynBrouwer/magister-school-integration)
2. **Configure your Magister account** in Home Assistant with your school, username, and password
3. **Verify the sensors** are created in Home Assistant (e.g., `sensor.magister_jan`)
4. **Add the plugin** to your Tessera system through the app
5. **Configure the child sensor** in the plugin settings:
   - Go to Plugins → Magister School
   - Under "Child sensor", select your child's Magister sensor
6. **Add tiles to your layout**:
   - Go to Layouts and edit your desired layout
   - Add Magister School tiles (Next lesson, Schedule, Homework, or Grades)
   - Position them as desired

## 📱 Available Tiles

### Next Lesson
Shows the next lesson with subject name, room, and countdown.

### Schedule
Shows the number of lessons today.

### Homework
Shows how many homework assignments are open.

### Grades
Shows recent grades or average.

## ⚙️ Configuration

In the Tessera app:
1. Go to **Plugins** → **Magister School**
2. Choose your child's sensor under **Child sensor**
3. Add tiles to your layout via **Layouts**

## 🎨 Examples

The plugin offers different tile sizes:
- **1x1 tile**: Compact overview
- **2x1 tile**: More details
- **3x2 tile**: Full schedule

## 🔧 Technical

This plugin reads data from Home Assistant entities created by the Magister School Integration. No direct API synchronization - all data comes via Home Assistant.

## 📝 Privacy

The plugin makes no network connections. All data is read from existing Home Assistant entities.

## 🛠️ Development

This plugin is made for the Tessera ecosystem. Contributions are welcome via issues and pull requests.

## 📄 License

MIT License - see the LICENSE file for details.

## ⚠️ Disclaimer

This plugin is not officially associated with Magister. Use at your own risk.
