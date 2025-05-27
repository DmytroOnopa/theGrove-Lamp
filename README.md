# theGrove🌳Lamp v2.0 🌈

*"Where custom LEDs meet quirky segment displays!"* ✨

## 🚀 Overview
The theGrove🌳Lamp is an RGB LED lamp featuring:

- 🎨 30x addressable LEDs (NeoPixel)  
- 🔣 *Crazy* 24-segment display  
- 🎛️ Encoder control with *satisfying clicks*  
- 🔊 Audio feedback (*beep-boop!*)  
- 💾 Saves your fav settings  


The standout feature is its unique segment display implementation that drives a non-standard indicator layout.

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
1. 🏃‍♂️ **Running Dot** - *Like a mini marathon!*  
2. 🕺 **Disco Swirl** - *Party mode activated*  
3. ☄️ **Meteor Shower** - *Space magic!*  

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

*(Include your specific segment wiring details here)*

## Usage

1. **Power on**: Lamp fades up to last saved color
2. **Rotate encoder**:
   - Normally adjusts hue
   - After single click, adjusts brightness
3. **Click encoder**:
   - Single: Toggle hue/brightness adjustment
   - Double: Cycle through lighting modes
4. **In Animation Mode**: Single click cycles effects
5. **In Emergency Mode**: Single click toggles flasher style

## Customization

- Adjust `HUE_STEP` in code for faster/slower hue changes
- Modify `numEffects` to add more animations
- Update segment patterns in `segment_driver.cpp`

## Troubleshooting

- **LEDs not lighting**: Check data line connection
- **Segment display issues**: Verify all segment pins are correctly mapped
- **Encoder problems**: Ensure pins are using INPUT_PULLUP

## Project Structure

- `theGroveLamp_latest.ino`: Main firmware
- `segment_driver.h/cpp`: Custom segment display driver
- `EEPROM.h`: For settings persistence

## License

[MIT License](LICENSE)

---

**Created by**: [DmytroOnopa]  
**Special Thanks**: [DeepSeek, OpenAI] for collaboration on the segment driver. 😀