# theGrove🌳Lamp v2.5 🌈

*"Now with lightsaber mode and plasma vortex effects!"* ✨⚔️

## 🚀 Overview
The theGrove🌳Lamp is an RGB LED lamp featuring:

- 🎨 30x addressable LEDs (NeoPixel)  
- 🔣 *Crazy* 24-segment display  
- � Encoder control with *satisfying clicks*  
- 🔊 Enhanced audio feedback (*beep-boop & saber sounds!*)  
- 💾 Saves your fav settings  
- ⚔️ **NEW** Lightsaber interactive mode  

The standout feature is its unique segment display and now includes a fully interactive lightsaber mode with motion effects.

Built because *normal lamps are boring* 😎

## 📦 Hardware Components
| Component              | Emoji | Qty |
|------------------------|-------|-----|
| Arduino Nano/UNO       | 🖥️   | 1   |
| WS2812B LED Strip      | 🌈    | 30  |
| KY-040 Encoder         | 🔄    | 1   |
| Piezo Buzzer           | 🔔    | 1   |
| KL1888CB Segm. Display | 🔠    | 1   |
| Resistors/Capacitors   | ⚡    | Few |

## 💡 Lighting Modes
### 🌟 **Normal Mode**
- Solid color *~vibes~*
- Twist encoder to change:  
  🌈 Hue (default)  
  ☀️ Brightness (after click)

### 🎪 **Animation Mode** *(Double-click to enter)*
1. 🏃‍♂️ **Running Dot** - *Smooth gradient marathon*  
2. 🌪️ **Rave Storm** - *Random color explosions*  
3. 🌀 **Color Tornado** - *Hypnotic spiral patterns*  
4. ⚡ **Plasma Vortex** - *NEW! Trippy wave effects*  

### ⚔️ **Interactive Lightsaber Mode**
- Power on/off with animation  
- Dynamic hum and swing sounds  
- Adjustable blade color  
- Motion-sensitive effects  

### 🚨 **Emergency Modes**  
- 👮 **Police Flash** *(Red/blue party!)*  
- 🚧 **Yellow/Blue Pulse** *(Construction vibe)*  

## 🔌 Wiring Guide
| Component       | Arduino Pin |
|-----------------|-------------|
| LED Strip Data  | 8           |
| Encoder CLK     | A0          |
| Encoder DT      | A1          |
| Encoder SW      | A2          |
| Buzzer          | 9           |
| Segment Pins    | 2-7         |

## 🎛️ Usage
1. **Power on**: Lamp fades up to last saved color
2. **Rotate encoder**:
   - Normally adjusts hue
   - After single click, adjusts brightness
3. **Click encoder**:
   - Single: Toggle hue/brightness adjustment
   - Double: Cycle through lighting modes
4. **Long press**: Change current mode
5. **In Lightsaber Mode**: 
   - Single click to activate/deactivate
   - Rotate to change blade color

## 🛠️ Customization
- Adjust `SABER_ANIMATION_SPEED` for faster/slower saber effects
- Modify `plasmaVortexEffect()` parameters for different wave patterns
- Update `LIGHTSABER_HUM_FREQ` for different saber sounds
- Change `numEffects` to add more animations

## 🐛 Troubleshooting
- **LEDs flickering**: Check power supply capacity
- **Saber sounds distorted**: Adjust buzzer frequencies
- **Encoder skipping steps**: Increase `debounceDelay`

## 📂 Project Structure
- `theGroveLampTest.ino`: Main firmware
- `segment_driver.h/cpp`: Custom segment display driver
- `FastLED.h`: For optimized LED control
- `EEPROM.h`: For settings persistence

## ✨ New in v2.5
- Added interactive lightsaber mode with sound effects
- 4 stunning animation effects including plasma vortex
- Smoother transitions and better EEPROM handling
- Enhanced audio feedback system
- Optimized power-up/power-down sequences

## License
[MIT License](LICENSE)

---

**Created by**: [DmytroOnopa]  
**Special Thanks**: [DeepSeek, OpenAI] for collaboration on the plasma vortex effects. 😀