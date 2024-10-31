# Smart Home Automation Project - Advanced Features in Action! 🏠⚙️

I'm excited to share my latest Smart Home Automation project built around the ATmega32 microcontroller! This system combines secure user access, intuitive control, and automated functionality to seamlessly manage home appliances.

## 🌟 Key Highlights

### 🔑 Secure Login System

- On startup, users input a username and password via the USART protocol through a laptop.
- Credentials can be updated anytime in the settings menu for flexible, secure access.

### 💡 LED and Switch Control

- Manual control of lights via commands or switches connected through XOR logic with the microcontroller pins.

### 🔄 Motor Speed Control

- **Manual Mode:** User-defined motor speed control.
- **Automatic Mode:** Motor speed adjusts dynamically with temperature (20°C to 45°C); higher temperature, higher speed.
- When automatic mode is off, speed reverts to the last manual setting.

### ⚙️ Servo Motor for Door Control

- Servo controls the door position: 90° locks the door, 0° unlocks it, enhancing security with automated control.

### ⚠️ Fire Alarm System

- Triggers when temperature exceeds 45°C: shuts off lights and activates an alarm until temperature drops.
- Fire alert displayed on LCD for immediate attention.

### ⏲️ Inactivity Timeout

- Standby mode after 2 seconds of inactivity, saving power and improving efficiency.

### 🔧 Settings Flexibility

- Users can update their username and password, ensuring ongoing security and customization.

## 📍 Reflecting My Passion

This project showcases my growing interest in embedded systems, and I’m excited to explore more innovations in automation and smart tech!

## Project Structure

### MCAL (Microcontroller Abstraction Layer)

- **DIO**
- **SPI**
- **USART**
- **TIMER**
- **EEPROM**
- **ADC**
- **INTERRUPT**

### HAL (Hardware Abstraction Layer)

- **Servo Motor**
- **DC Motors**
- **LEDs**
- **CLCD**
- **Buzzer**
- **LM35 Temperature Sensor**

### GitHub Repository

[Smart Home Automation Project Video](https://www.linkedin.com/posts/abdallah-shehawey_smarthome-automation-embeddedsystems-activity-7254262439783899136-0IPK?utm_source=share&utm_medium=member_desktop)

Looking forward to connecting with others interested in smart home solutions and embedded engineering!
