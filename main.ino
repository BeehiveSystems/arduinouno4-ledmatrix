/**
 * Hello World for Arduino UNO R4 WiFi LED Matrix with Web Control
 *
 * Features:
 * - Uses built-in 12x8 LED matrix on the Arduino UNO R4 WiFi.
 * - Connects to Wi-Fi using provided credentials.
 * - Creates a local web server hosting an interactive page.
 * - The webpage displays a 12x8 grid of clickable buttons representing each LED.
 * - Clicking a button toggles the corresponding LED on/off.
 * - Uses AJAX (fetch API) for real-time LED state updates without page reload.
 * - LED states are stored in an 8x12 boolean array.
 * - On/off states are reflected in the LED matrix and on the webpage’s buttons.
 * - Shows device’s IP address on both the Serial Monitor and webpage.
 * - Attempts Wi-Fi connection up to 20 times before giving up.
 */

#include <Arduino.h>
#include <WiFiS3.h>
#include <Arduino_LED_Matrix.h>
#include "config.h"

// Dimensions of the LED matrix
static const int LED_COLS = 12;
static const int LED_ROWS = 8;

// Global array to store LED states (row-major: ledStates[row][col])
bool ledStates[LED_ROWS][LED_COLS] = { false };

// Start the matrix
ArduinoLEDMatrix MATRIX;

// Web server on port 80
WiFiServer server(80);

// Forward declarations
void updateMatrix();
String generateHTMLPage(String ip);
String generateStatesJSON();
void handleRequest(WiFiClient client, String request);

void setup() {
  Serial.begin(115200);

  // Initialize LED Matrix
  MATRIX.begin();
  updateMatrix(); // Show initial empty state

  // Connect to WiFi
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  int attemptCount = 0;
  while (WiFi.status() != WL_CONNECTED && attemptCount < 20) {
    delay(500);
    Serial.print(".");
    attemptCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi.");
    return; // Stop if no Wi-Fi
  }

  // Start the server
  server.begin();
  Serial.println("Web server started.");
  Serial.print("Server IP: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  while(!client.available()){
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  client.readStringUntil('\n'); // discard rest of the line

  // Handle the request
  handleRequest(client, request);

  delay(1);
  client.stop();
}

// Update the LED matrix based on ledStates
void updateMatrix() {
  static byte frame[8][12];  // Create a 2D byte array for the frame
  
  // Fill the frame array based on ledStates
  for (int y = 0; y < LED_ROWS; y++) {
    for (int x = 0; x < LED_COLS; x++) {
      frame[y][x] = ledStates[y][x] ? 1 : 0;
    }
  }
  
  // Update the matrix with the new frame
  MATRIX.renderBitmap(frame, 8, 12);
}

// Generate the main HTML page
String generateHTMLPage(String ip) {
  String page = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0"/>
<title>UNO R4 WiFi LED Matrix</title>
<style>
  body {
    font-family: sans-serif;
    background: #1e1e1e;
    color: #ccc;
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 20px;
  }
  h1, h2 {
    color: #eee;
  }
  .grid {
    display: grid;
    grid-template-columns: repeat(12, 30px);
    grid-template-rows: repeat(8, 30px);
    gap: 5px;
    margin-bottom: 20px;
  }
  .cell {
    width: 30px;
    height: 30px;
    border-radius: 4px;
    background: #333;
    cursor: pointer;
    transition: background-color 0.3s, box-shadow 0.3s;
    display: flex;
    align-items: center;
    justify-content: center;
  }
  .cell.on {
    background: yellow;
    box-shadow: 0 0 10px yellow;
  }
  #ipaddress {
    margin-top: 20px;
    color: #eee;
  }
  .buttons {
    margin-top: 20px;
  }
  .buttons button {
    margin: 0 10px;
    padding: 10px 20px;
    font-size: 16px;
    cursor: pointer;
  }
</style>
</head>
<body>
<h1>UNO R4 WiFi LED Matrix Control</h1>
<div class="grid" id="ledGrid"></div>
<div id="ipaddress">IP Address: )rawliteral";

  page += ip;
  page += R"rawliteral(</div>
<div class="buttons">
  <button id="lightAll">Light All</button>
  <button id="clearAll">Clear All</button>
</div>
<script>
// Build the grid
const rows = 8;
const cols = 12;
const grid = document.getElementById('ledGrid');

for (let y = 0; y < rows; y++){
  for (let x = 0; x < cols; x++){
    const cell = document.createElement('div');
    cell.classList.add('cell');
    cell.dataset.x = x;
    cell.dataset.y = y;
    cell.addEventListener('click', () => toggleLED(x, y, cell));
    grid.appendChild(cell);
  }
}

// Fetch current states from server
async function loadStates() {
  try {
    const resp = await fetch('/state');
    const json = await resp.json();
    updateGrid(json.states);
  } catch (e) {
    console.error(e);
  }
}

function updateGrid(states) {
  const cells = document.querySelectorAll('.cell');
  cells.forEach(cell => {
    const x = parseInt(cell.dataset.x);
    const y = parseInt(cell.dataset.y);
    if (states[y][x]) {
      cell.classList.add('on');
    } else {
      cell.classList.remove('on');
    }
  });
}

async function toggleLED(x, y, cell) {
  try {
    const resp = await fetch(`/toggle?x=${x}&y=${y}`);
    const json = await resp.json();
    // Update just this cell based on response
    if (json.state === true) {
      cell.classList.add('on');
    } else {
      cell.classList.remove('on');
    }
  } catch (e) {
    console.error(e);
  }
}

// Add event listeners for "Light All" and "Clear All" buttons
document.getElementById('lightAll').addEventListener('click', async () => {
  try {
    await fetch('/lightall');
    loadStates(); // Refresh the grid
  } catch (e) {
    console.error(e);
  }
});

document.getElementById('clearAll').addEventListener('click', async () => {
  try {
    await fetch('/clearall');
    loadStates(); // Refresh the grid
  } catch (e) {
    console.error(e);
  }
});

// Load initial states
loadStates();
</script>
</body>
</html>
)rawliteral";

  return page;
}

// Generate JSON state response
String generateStatesJSON() {
  String json = "{\"states\":[";
  for (int y = 0; y < LED_ROWS; y++) {
    json += "[";
    for (int x = 0; x < LED_COLS; x++) {
      json += (ledStates[y][x] ? "true" : "false");
      if (x < LED_COLS - 1) json += ",";
    }
    json += "]";
    if (y < LED_ROWS - 1) json += ",";
  }
  json += "]}";
  return json;
}

// Handle HTTP requests
void handleRequest(WiFiClient client, String request) {
  if (request.startsWith("GET / ")) {
    // Serve main page
    String html = generateHTMLPage(WiFi.localIP().toString());
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(html);
  }
  else if (request.startsWith("GET /toggle")) {
    // Parse parameters x and y
    int xIndex = request.indexOf("x=");
    int yIndex = request.indexOf("y=");
    if (xIndex > -1 && yIndex > -1) {
      int ampIndex = request.indexOf('&', xIndex);
      String xVal = request.substring(xIndex+2, ampIndex);
      String yVal = request.substring(yIndex+2, request.indexOf(' ', yIndex));

      int x = xVal.toInt();
      int y = yVal.toInt();

      if (x >= 0 && x < LED_COLS && y >= 0 && y < LED_ROWS) {
        ledStates[y][x] = !ledStates[y][x];
        updateMatrix();
        // Return JSON with the new state
        String json = "{\"x\":" + String(x) + ",\"y\":" + String(y) + ",\"state\":" + (ledStates[y][x] ? "true" : "false") + "}";

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println();
        client.println(json);
      } else {
        // Out of range
        client.println("HTTP/1.1 400 Bad Request");
        client.println("Connection: close");
        client.println();
      }
    } else {
      // Missing parameters
      client.println("HTTP/1.1 400 Bad Request");
      client.println("Connection: close");
      client.println();
    }
  }
  else if (request.startsWith("GET /state")) {
    // Return the entire state array
    String json = generateStatesJSON();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(json);
  }
  else if (request.startsWith("GET /lightall")) {
    // Light all LEDs
    for (int y = 0; y < LED_ROWS; y++) {
      for (int x = 0; x < LED_COLS; x++) {
        ledStates[y][x] = true;
      }
    }
    updateMatrix();
    client.println("HTTP/1.1 200 OK");
    client.println("Connection: close");
    client.println();
  }
  else if (request.startsWith("GET /clearall")) {
    // Clear all LEDs
    for (int y = 0; y < LED_ROWS; y++) {
      for (int x = 0; x < LED_COLS; x++) {
        ledStates[y][x] = false;
      }
    }
    updateMatrix();
    client.println("HTTP/1.1 200 OK");
    client.println("Connection: close");
    client.println();
  }
  else {
    // Not found
    client.println("HTTP/1.1 404 Not Found");
    client.println("Connection: close");
    client.println();
  }
}
