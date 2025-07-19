#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

// Configuration
const char *AP_SSID = "Olara Emmanuel";
const char *AP_PASSWORD = "Em.ma.45"; // Min 8 characters
const byte DNS_PORT = 53;
const char *HOSTNAME = "lighting-control";

// Security configuration
#define MAX_LOGIN_ATTEMPTS 5
#define LOCKOUT_TIME 300000 // 5 minutes

// LED pins
#define RED_LED 23
#define GREEN_LED 22
#define ORANGE_LED 21

// Security states
int loginAttempts = 0;
unsigned long lockoutStart = 0;
bool isLocked = false;

// LED states
bool redState = false;
bool greenState = false;
bool orangeState = false;

WebServer server(80);
DNSServer dnsServer;
Preferences preferences;

// HTML content for the captive portal and dashboard
const char *captivePortal = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Secure Lighting System</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #1a2a6c, #b21f1f);
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
      color: white;
    }
    
    .login-container {
      background: rgba(0, 0, 0, 0.7);
      backdrop-filter: blur(10px);
      border-radius: 15px;
      padding: 30px;
      width: 350px;
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5);
      text-align: center;
    }
    
    .logo {
      font-size: 2.5rem;
      margin-bottom: 20px;
      color: #4CAF50;
    }
    
    h1 {
      margin-bottom: 30px;
      font-weight: 300;
      letter-spacing: 1px;
    }
    
    .instructions {
      background: rgba(255, 255, 255, 0.1);
      border-radius: 10px;
      padding: 15px;
      margin-bottom: 20px;
      text-align: left;
      font-size: 0.9rem;
    }
    
    .instructions ol {
      padding-left: 20px;
    }
    
    .instructions li {
      margin-bottom: 10px;
    }
    
    .input-group {
      margin-bottom: 20px;
      text-align: left;
    }
    
    label {
      display: block;
      margin-bottom: 8px;
      font-weight: 500;
    }
    
    input {
      width: 100%;
      padding: 12px;
      border: none;
      border-radius: 8px;
      background: rgba(255, 255, 255, 0.1);
      color: white;
      font-size: 16px;
      border: 1px solid rgba(255, 255, 255, 0.2);
    }
    
    button {
      width: 100%;
      padding: 14px;
      background: linear-gradient(to right, #4CAF50, #2E7D32);
      border: none;
      border-radius: 8px;
      color: white;
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s;
      margin-top: 10px;
    }
    
    button:hover {
      transform: translateY(-2px);
      box-shadow: 0 5px 15px rgba(0, 0, 0, 0.3);
    }
    
    .error-message {
      color: #ff5252;
      margin-top: 15px;
      font-weight: 500;
      display: none;
    }
    
    .lockout-message {
      color: #ff9800;
      margin-top: 20px;
      font-weight: 500;
      display: none;
    }
    
    .footer {
      margin-top: 30px;
      font-size: 0.85rem;
      color: rgba(255, 255, 255, 0.6);
    }
  </style>
</head>
<body>
  <div class="login-container">
    <div class="logo">
      <i class="fas fa-lightbulb"></i>
    </div>
    <h1>Secure Lighting Control</h1>
    
    <div class="instructions">
      <p><strong>Connection Instructions:</strong></p>
      <ol>
        <li>Stay connected to this WiFi network: <strong>SecureLightingSystem</strong></li>
        <li>Enter your credentials to access the control panel</li>
        <li>After login, you can control the lights</li>
        <li>Keep this page open while using the system</li>
      </ol>
    </div>
    
    <form id="loginForm">
      <div class="input-group">
        <label for="username">Username</label>
        <input type="text" id="username" name="username" required autocomplete="off">
      </div>
      
      <div class="input-group">
        <label for="password">Password</label>
        <input type="password" id="password" name="password" required>
      </div>
      
      <button type="submit">Login</button>
      
      <div class="error-message" id="errorMessage">
        Invalid credentials. Please try again.
      </div>
      
      <div class="lockout-message" id="lockoutMessage">
        System locked. Try again in <span id="countdown">5:00</span> minutes.
      </div>
    </form>
    
    <div class="footer">
      <p>&copy; 2024 [Your Name] Lighting Systems | Team: [Your Team Name]</p>
    </div>
  </div>

  <script>
    document.getElementById('loginForm').addEventListener('submit', function(e) {
      e.preventDefault();
      
      const username = document.getElementById('username').value;
      const password = document.getElementById('password').value;
      
      fetch('/login', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: `username=${encodeURIComponent(username)}&password=${encodeURIComponent(password)}`
      })
      .then(response => {
        if (response.ok) {
          window.location.href = '/dashboard';
        } else {
          return response.text();
        }
      })
      .then(error => {
        document.getElementById('errorMessage').style.display = 'block';
        if (error === "lockout") {
          document.getElementById('lockoutMessage').style.display = 'block';
          document.getElementById('errorMessage').style.display = 'none';
          startCountdown(300); // 5 minutes
        }
      })
      .catch(error => {
        console.error('Error:', error);
        document.getElementById('errorMessage').style.display = 'block';
      });
    });
    
    function startCountdown(seconds) {
      const countdownElement = document.getElementById('countdown');
      let remaining = seconds;
      
      const interval = setInterval(() => {
        const minutes = Math.floor(remaining / 60);
        const secs = remaining % 60;
        
        countdownElement.textContent = `${minutes}:${secs < 10 ? '0' : ''}${secs}`;
        
        if (remaining <= 0) {
          clearInterval(interval);
          document.getElementById('lockoutMessage').style.display = 'none';
        }
        
        remaining--;
      }, 1000);
    }
  </script>
  
  <script src="https://kit.fontawesome.com/a076d05399.js" crossorigin="anonymous"></script>
</body>
</html>
)rawliteral";

const char *dashboardPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Secure Lighting Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    /* (Keep the same dashboard styles as before) */
  </style>
</head>
<body>
  <!-- Security Header -->
  <div class="security-header">
    <div class="security-status">
      <div class="security-icon"></div>
      <span>Secure Connection • Access Point Mode</span>
    </div>
    <button class="logout-btn" onclick="logout()">
      <i class="fas fa-sign-out-alt"></i> Logout
    </button>
  </div>
  
  <!-- Main Dashboard -->
  <div class="dashboard">
    <div class="dashboard-header">
      <h1 class="dashboard-title">Secure Lighting Control</h1>
      <p class="dashboard-subtitle">Direct Access Mode - No Internet Required</p>
    </div>
    
    <div class="cards-grid">
      <!-- LED Control Card -->
      <div class="card">
        <div class="card-header">
          <div class="card-icon">
            <i class="fas fa-lightbulb"></i>
          </div>
          <h2 class="card-title">Lighting Control</h2>
        </div>
        
        <div class="led-control">
          <!-- Red LED -->
          <div class="led-item">
            <div class="led-info">
              <div class="led-color red"></div>
              <div class="led-name">Red Light</div>
            </div>
            <label class="toggle-switch">
              <input type="checkbox" id="red-led" onchange="toggleLED('red')">
              <span class="slider"></span>
            </label>
          </div>
          
          <!-- Green LED -->
          <div class="led-item">
            <div class="led-info">
              <div class="led-color green"></div>
              <div class="led-name">Green Light</div>
            </div>
            <label class="toggle-switch">
              <input type="checkbox" id="green-led" onchange="toggleLED('green')">
              <span class="slider"></span>
            </label>
          </div>
          
          <!-- Orange LED -->
          <div class="led-item">
            <div class="led-info">
              <div class="led-color orange"></div>
              <div class="led-name">Orange Light</div>
            </div>
            <label class="toggle-switch">
              <input type="checkbox" id="orange-led" onchange="toggleLED('orange')">
              <span class="slider"></span>
            </label>
          </div>
        </div>
      </div>
      
      <!-- Connection Card -->
      <div class="card">
        <div class="card-header">
          <div class="card-icon">
            <i class="fas fa-wifi"></i>
          </div>
          <h2 class="card-title">Connection Info</h2>
        </div>
        
        <div class="info-grid">
          <div class="info-item">
            <div class="info-icon">
              <i class="fas fa-network-wired"></i>
            </div>
            <div class="info-text">
              <h3>Connection Mode</h3>
              <p>Direct Access Point</p>
            </div>
          </div>
          
          <div class="info-item">
            <div class="info-icon">
              <i class="fas fa-signal"></i>
            </div>
            <div class="info-text">
              <h3>Network Name</h3>
              <p>SecureLightingSystem</p>
            </div>
          </div>
          
          <div class="info-item">
            <div class="info-icon">
              <i class="fas fa-info-circle"></i>
            </div>
            <div class="info-text">
              <h3>Device IP</h3>
              <p>192.168.4.1</p>
            </div>
          </div>
          
          <div class="info-item">
            <div class="info-icon">
              <i class="fas fa-lock"></i>
            </div>
            <div class="info-text">
              <h3>Security</h3>
              <p>WPA2 Protected</p>
            </div>
          </div>
        </div>
      </div>
      
      <!-- System Info Card -->
      <div class="card">
        <div class="card-header">
          <div class="card-icon">
            <i class="fas fa-microchip"></i>
          </div>
          <h2 class="card-title">System Information</h2>
        </div>
        
        <div class="info-grid">
          <div class="info-item">
            <div class="info-icon">
              <i class="fas fa-memory"></i>
            </div>
            <div class="info-text">
              <h3>Memory Usage</h3>
              <p>42%</p>
            </div>
          </div>
          
          <div class="info-item">
            <div class="info-icon">
              <i class="fas fa-temperature-high"></i>
            </div>
            <div class="info-text">
              <h3>System Temperature</h3>
              <p>42°C</p>
            </div>
          </div>
          
          <div class="info-item">
            <div class="info-icon">
              <i class="fas fa-power-off"></i>
            </div>
            <div class="info-text">
              <h3>Uptime</h3>
              <p id="uptime">00:15:32</p>
            </div>
          </div>
          
          <div class="info-item">
            <div class="info-icon">
              <i class="fas fa-user"></i>
            </div>
            <div class="info-text">
              <h3>Active Users</h3>
              <p>1</p>
            </div>
          </div>
        </div>
      </div>
    </div>
    
    <div class="footer">
      <p>&copy; 2024 [Your Name] Lighting Systems | Team: [Your Team Name]</p>
    </div>
  </div>

  <script>
    // Initialize LED states on page load
    window.onload = function() {
      fetch('/getLedStates')
        .then(response => response.json())
        .then(data => {
          document.getElementById('red-led').checked = data.red;
          document.getElementById('green-led').checked = data.green;
          document.getElementById('orange-led').checked = data.orange;
        });
      
      // Update uptime every second
      setInterval(updateUptime, 1000);
    };
    
    // Toggle LED state
    function toggleLED(color) {
      const checkbox = document.getElementById(`${color}-led`);
      const state = checkbox.checked;
      
      fetch(`/setLed?color=${color}&state=${state}`, {
        method: 'POST',
        headers: {
          'Authorization': 'Bearer ' + localStorage.getItem('authToken')
        }
      })
      .then(response => {
        if (!response.ok) {
          checkbox.checked = !state;
        }
      })
      .catch(error => {
        console.error('Error:', error);
        checkbox.checked = !state;
      });
    }
    
    // Update uptime display
    function updateUptime() {
      const uptimeElement = document.getElementById('uptime');
      const time = uptimeElement.textContent.split(':');
      let hours = parseInt(time[0]);
      let minutes = parseInt(time[1]);
      let seconds = parseInt(time[2]);
      
      seconds++;
      if (seconds >= 60) {
        seconds = 0;
        minutes++;
        if (minutes >= 60) {
          minutes = 0;
          hours++;
        }
      }
      
      uptimeElement.textContent = 
        `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
    }
    
    // Logout function
    function logout() {
      localStorage.removeItem('authToken');
      window.location.href = '/';
    }
  </script>
  
  <script src="https://kit.fontawesome.com/a076d05399.js" crossorigin="anonymous"></script>
</body>
</html>
)rawliteral";

void setup()
{
  Serial.begin(115200);

  // Initialize LED pins
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(ORANGE_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(ORANGE_LED, LOW);

  // Initialize preferences
  preferences.begin("auth", false);

  // Start Access Point
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress apIP = WiFi.softAPIP();
  Serial.println("Access Point Started");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);
  Serial.print("IP Address: ");
  Serial.println(apIP);

  // Start DNS server for captive portal
  dnsServer.start(DNS_PORT, "*", apIP);

  // Set up server routes
  server.on("/", HTTP_GET, []()
            {
    if (isLocked) {
      server.send(200, "text/html", getLockoutPage());
    } else {
      server.send(200, "text/html", captivePortal);
    } });

  server.on("/login", HTTP_POST, []()
            {
    // Check if system is locked
    if (isLocked) {
      server.send(403, "text/plain", "System locked");
      return;
    }
    
    // Get credentials from form
    String username = server.arg("username");
    String password = server.arg("password");
    
    // Validate credentials
    if (authenticate(username, password)) {
      loginAttempts = 0;
      String token = generateToken();
      server.sendHeader("Location", "/dashboard");
      server.sendHeader("Set-Cookie", "authToken=" + token + "; HttpOnly");
      server.send(302);
    } else {
      loginAttempts++;
      if (loginAttempts >= MAX_LOGIN_ATTEMPTS) {
        isLocked = true;
        lockoutStart = millis();
        server.send(401, "text/plain", "lockout");
      } else {
        server.send(401, "text/plain", "Invalid credentials");
      }
    } });

  server.on("/dashboard", HTTP_GET, []()
            {
    if (!isAuthenticated()) {
      server.sendHeader("Location", "/");
      server.send(302);
      return;
    }
    server.send(200, "text/html", dashboardPage); });

  server.on("/getLedStates", HTTP_GET, []()
            {
    if (!isAuthenticated()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    String json = "{";
    json += "\"red\":" + String(redState ? "true" : "false") + ",";
    json += "\"green\":" + String(greenState ? "true" : "false") + ",";
    json += "\"orange\":" + String(orangeState ? "true" : "false");
    json += "}";
    
    server.send(200, "application/json", json); });

  server.on("/setLed", HTTP_POST, []()
            {
    if (!isAuthenticated()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    String color = server.arg("color");
    String state = server.arg("state");
    
    if (color == "red") {
      digitalWrite(RED_LED, state == "true" ? HIGH : LOW);
      redState = (state == "true");
    } else if (color == "green") {
      digitalWrite(GREEN_LED, state == "true" ? HIGH : LOW);
      greenState = (state == "true");
    } else if (color == "orange") {
      digitalWrite(ORANGE_LED, state == "true" ? HIGH : LOW);
      orangeState = (state == "true");
    }
    
    server.send(200, "text/plain", "OK"); });

  server.on("/logout", HTTP_GET, []()
            {
    server.sendHeader("Set-Cookie", "authToken=; expires=Thu, 01 Jan 1970 00:00:00 GMT");
    server.sendHeader("Location", "/");
    server.send(302); });

  // Handle captive portal redirects
  server.onNotFound([]()
                    {
    server.sendHeader("Location", "http://192.168.4.1");
    server.send(302); });

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  dnsServer.processNextRequest();
  server.handleClient();

  // Check lockout status
  if (isLocked && (millis() - lockoutStart) > LOCKOUT_TIME)
  {
    isLocked = false;
    loginAttempts = 0;
  }
}

// Authentication functions
bool authenticate(String username, String password)
{
  // Use these credentials:
  return (username == "Emmanuel" && password == "Em.ma.45");
}

String generateToken()
{
  return String(esp_random(), HEX) + String(millis(), HEX);
}

bool isAuthenticated()
{
  // Check for auth token in cookies
  String cookie = server.header("Cookie");
  if (cookie.indexOf("authToken=") != -1)
  {
    return true;
  }
  return false;
}

String getLockoutPage()
{
  String page = "<html><body style='background:#121212;color:#fff;text-align:center;padding:50px;'>";
  page += "<h1>System Locked</h1>";
  page += "<p>Too many failed login attempts. Please try again later.</p>";
  page += "<p>Time remaining: <span id='countdown'>05:00</span></p>";
  page += "<script>";
  page += "let time = 300;";
  page += "function update() {";
  page += "  const min = Math.floor(time/60).toString().padStart(2,'0');";
  page += "  const sec = (time % 60).toString().padStart(2,'0');";
  page += "  document.getElementById('countdown').textContent = `${min}:${sec}`;";
  page += "  if (time-- <= 0) location.reload();";
  page += "}";
  page += "setInterval(update, 1000);";
  page += "</script>";
  page += "</body></html>";
  return page;
}
