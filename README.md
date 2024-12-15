# **Arduino UNO R4 WiFi LED Matrix Control**

Control the built-in 12x8 LED matrix on the Arduino UNO R4 WiFi via a web interface. Toggle individual LEDs, light up all LEDs, or clear the matrix.

----------

## **Features**

-   **Web Interface**:
    
    -   12x8 grid of buttons to toggle individual LEDs.
        
    -   "Light All" and "Clear All" buttons.
        
-   **Real-Time Updates**:
    
    -   LED states are updated in real-time using AJAX.
        
-   **Wi-Fi Configuration**:
    
    -   Wi-Fi credentials are stored in a separate `config.h` file.
        

----------

## **Setup**

### **1. Clone the Repository**

```
git clone https://github.com/your-username/arduino-uno-r4-wifi-led-matrix.git
cd arduino-uno-r4-wifi-led-matrix
```

### **2. Configure Wi-Fi**

1.  Create a `config.h` file:
    
```
// config.h
const char* ssid = "Your_SSID";
const char* password = "Your_Password";
```
    
2.  Add `config.h` to `.gitignore`:

```
config.h
```
    

### **3. Upload the Code**

1.  Open `led_matrix_control.ino` in the Arduino IDE.
    
2.  Select the **Arduino UNO R4 WiFi** board.
    
3.  Upload the code to the board.
    

### **4. Access the Web Interface**

1.  Open the Serial Monitor (baud rate: 115200).
    
2.  Note the device's IP address.
    
3.  Navigate to the IP address in a web browser (e.g., `http://192.168.1.100`).
    

----------

## **Usage**

-   **LED Grid**:
    
    -   Click buttons to toggle LEDs.
        
-   **Light All**:
    
    -   Turn on all LEDs.
        
-   **Clear All**:
    
    -   Turn off all LEDs.
        

----------

## **Project Structure**

```
arduino-uno-r4-wifi-led-matrix/
├── led_matrix_control.ino
├── config.h
├── README.md
```

----------

