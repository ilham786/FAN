// ================================================================
//  PROJECT   : Wireless Fan Switch Controller — Dual Servo v2.0
//  HARDWARE  : NodeMCU ESP8266 + 2x SG90 Servo Motors
//              Servo 1 (D1 / GPIO5) -> dedicated to pressing ON
//              Servo 2 (D2 / GPIO4) -> dedicated to pressing OFF
//  CONTROL   : Wi-Fi Web Server  (no app needed, use any browser)
//  DEVELOPER : ILHAM FAROOQUE
//  GITHUB    : https://github.com/ilham786
//  INSTAGRAM : https://www.instagram.com/ilham_farooque/
// ================================================================

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// ----------------------------------------------------------------
//  STEP 1 - Set your Wi-Fi credentials before uploading
// ----------------------------------------------------------------
const char* WIFI_SSID     = "Oxyzone";
const char* WIFI_PASSWORD = "oxyzone@321";

// ----------------------------------------------------------------
//  STEP 2 - Servo pin assignment
//  Servo 1 signal wire -> D1 pin on NodeMCU  (ON  servo)
//  Servo 2 signal wire -> D2 pin on NodeMCU  (OFF servo)
// ----------------------------------------------------------------
const int SERVO1_PIN = D1;   // GPIO5
const int SERVO2_PIN = D2;   // GPIO4

// ----------------------------------------------------------------
//  STEP 3 - Angle calibration
//  After mounting servos physically, adjust these angles in 5-deg
//  steps until the switch clicks reliably in both directions.
// ----------------------------------------------------------------
const int S1_NEUTRAL = 90;   // Servo 1 rest  (not touching switch)
const int S1_PRESS   = 55;   // Servo 1 press (pushes switch ON)
const int S2_NEUTRAL = 90;   // Servo 2 rest  (not touching switch)
const int S2_PRESS   = 125;  // Servo 2 press (pushes switch OFF)

// Timing
const int PRESS_HOLD_MS   = 450;  // ms to hold switch pressed
const int RETURN_DELAY_MS = 200;  // ms pause after returning

// ================================================================
//  GLOBAL OBJECTS AND STATE
// ================================================================
ESP8266WebServer server(80);
Servo servoON;    // Servo 1  -  turns fan ON
Servo servoOFF;   // Servo 2  -  turns fan OFF
bool fanIsOn = false;
bool isBusy  = false;

// ================================================================
//  WEB DASHBOARD  (served from ESP8266 flash via PROGMEM)
// ================================================================
const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Fan Controller</title>
  <link href="https://fonts.googleapis.com/css2?family=Space+Mono:wght@400;700&family=DM+Sans:wght@300;500;700&display=swap" rel="stylesheet">
  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

    :root {
      --bg:        #080c14;
      --surface:   #0f1724;
      --card:      #141e2e;
      --border:    #1e2d42;
      --muted:     #4a6080;
      --text:      #dce8f5;
      --soft:      #8fa8c4;
      --green:     #00e676;
      --green-dim: rgba(0,230,118,0.12);
      --red:       #ff5252;
      --red-dim:   rgba(255,82,82,0.12);
      --blue:      #448aff;
      --blue-dim:  rgba(68,138,255,0.1);
    }

    body {
      font-family: 'DM Sans', sans-serif;
      background: var(--bg);
      background-image:
        linear-gradient(rgba(68,138,255,0.03) 1px, transparent 1px),
        linear-gradient(90deg, rgba(68,138,255,0.03) 1px, transparent 1px);
      background-size: 40px 40px;
      color: var(--text);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }

    .card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 20px;
      width: 100%; max-width: 400px;
      overflow: hidden;
      box-shadow: 0 30px 80px rgba(0,0,0,0.6);
    }

    /* shimmer top bar */
    .accent-bar {
      height: 3px;
      background: linear-gradient(90deg, var(--blue), var(--green), var(--blue));
      background-size: 200% 100%;
      animation: shimmer 3s linear infinite;
    }
    @keyframes shimmer { to { background-position: 200% 0; } }

    /* header */
    .header {
      padding: 22px 26px 0;
      display: flex; align-items: center; justify-content: space-between;
    }
    .header h1 {
      font-family: 'Space Mono', monospace;
      font-size: 14px; font-weight: 700;
      color: var(--text); letter-spacing: 0.5px;
    }
    .header p { font-size: 10px; color: var(--muted); margin-top: 2px; font-family: 'Space Mono', monospace; }
    .wifi-pill {
      display: flex; align-items: center; gap: 5px;
      background: var(--blue-dim); border: 1px solid rgba(68,138,255,0.2);
      border-radius: 100px; padding: 4px 10px;
      font-size: 9px; color: var(--blue);
      font-family: 'Space Mono', monospace; letter-spacing: 0.5px;
    }
    .wifi-dot {
      width: 6px; height: 6px; border-radius: 50%;
      background: var(--blue); animation: pulse 2s ease infinite;
    }
    @keyframes pulse { 0%,100%{opacity:1;transform:scale(1)} 50%{opacity:0.4;transform:scale(0.8)} }

    /* fan ring */
    .fan-section { padding: 26px 26px 18px; display: flex; flex-direction: column; align-items: center; }
    .fan-outer {
      width: 128px; height: 128px; border-radius: 50%;
      display: flex; align-items: center; justify-content: center;
      border: 1px solid var(--border); background: var(--surface);
      transition: all 0.6s ease; position: relative;
    }
    .fan-outer.on {
      border-color: var(--green); background: var(--green-dim);
      box-shadow: 0 0 40px rgba(0,230,118,0.25), inset 0 0 20px rgba(0,230,118,0.08);
    }
    .fan-outer.off {
      border-color: var(--red); background: var(--red-dim);
      box-shadow: 0 0 30px rgba(255,82,82,0.12);
    }
    .fan-emoji { font-size: 50px; }
    .fan-outer.on .fan-emoji { animation: spin 1.2s linear infinite; }
    @keyframes spin { to { transform: rotate(360deg); } }

    /* status badge */
    .status-wrap { margin-top: 14px; }
    .status-badge {
      display: inline-flex; align-items: center; gap: 7px;
      padding: 6px 18px; border-radius: 100px;
      font-family: 'Space Mono', monospace;
      font-size: 11px; font-weight: 700; letter-spacing: 1px;
      transition: all 0.4s;
    }
    .status-badge.on  { background: var(--green-dim); color: var(--green); border: 1px solid rgba(0,230,118,0.3); }
    .status-badge.off { background: rgba(74,96,128,0.15); color: var(--muted); border: 1px solid var(--border); }
    .s-dot { width: 7px; height: 7px; border-radius: 50%; background: currentColor; }
    .status-badge.on .s-dot { animation: pulse 1.5s ease infinite; }

    /* servo chips */
    .servo-row { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; padding: 0 26px 18px; }
    .servo-chip {
      background: var(--surface); border: 1px solid var(--border);
      border-radius: 12px; padding: 12px; transition: all 0.35s; position: relative; overflow: hidden;
    }
    .servo-chip::before {
      content:''; position: absolute; top:0; left:0; right:0;
      height: 2px; background: var(--border); transition: all 0.35s;
    }
    .servo-chip.fon::before  { background: var(--green); box-shadow: 0 0 8px var(--green); }
    .servo-chip.foff::before { background: var(--red);   box-shadow: 0 0 8px var(--red); }
    .servo-chip.fon  { border-color: rgba(0,230,118,0.3); background: var(--green-dim); }
    .servo-chip.foff { border-color: rgba(255,82,82,0.3); background: var(--red-dim); }

    .chip-head { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
    .chip-num { font-family: 'Space Mono', monospace; font-size: 9px; font-weight: 700; letter-spacing: 1px; color: var(--muted); }
    .chip-led { width: 7px; height: 7px; border-radius: 50%; background: var(--border); transition: all 0.35s; }
    .servo-chip.fon  .chip-led { background: var(--green); box-shadow: 0 0 6px var(--green); animation: pulse 0.8s ease infinite; }
    .servo-chip.foff .chip-led { background: var(--red);   box-shadow: 0 0 6px var(--red);   animation: pulse 0.8s ease infinite; }

    .chip-title { font-size: 12px; font-weight: 700; color: var(--soft); margin-bottom: 3px; }
    .servo-chip.fon  .chip-title { color: var(--green); }
    .servo-chip.foff .chip-title { color: var(--red); }
    .chip-state { font-family: 'Space Mono', monospace; font-size: 10px; color: var(--muted); }
    .chip-pin {
      margin-top: 7px; display: inline-block;
      background: rgba(68,138,255,0.1); border: 1px solid rgba(68,138,255,0.15);
      border-radius: 4px; padding: 2px 7px;
      font-family: 'Space Mono', monospace; font-size: 9px; color: var(--blue);
    }

    /* buttons */
    .btn-section { padding: 0 26px; }
    .btn-row { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; }
    .btn {
      border: none; border-radius: 14px; padding: 18px 10px;
      font-family: 'DM Sans', sans-serif; font-size: 14px; font-weight: 700;
      cursor: pointer; transition: all 0.2s ease; position: relative; overflow: hidden;
    }
    .btn:active { transform: scale(0.95); }
    .btn:disabled { opacity: 0.3; cursor: not-allowed; transform: none; }
    .btn-on  { background: linear-gradient(155deg,#00c853,#00e676); color: #001a0a; box-shadow: 0 6px 24px rgba(0,230,118,0.35); }
    .btn-off { background: linear-gradient(155deg,#c62828,#ff5252); color: #fff;    box-shadow: 0 6px 24px rgba(255,82,82,0.3); }

    .hint-row { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-top: 7px; }
    .hint { text-align: center; font-family: 'Space Mono', monospace; font-size: 9px; color: var(--muted); }

    /* info strip */
    .info-strip {
      margin: 18px 26px 0;
      background: var(--surface); border: 1px solid var(--border);
      border-radius: 10px; padding: 10px 14px;
      display: flex; justify-content: space-between; align-items: center;
    }
    .info-item { font-family: 'Space Mono', monospace; font-size: 9px; color: var(--muted); display: flex; flex-direction: column; gap: 2px; align-items: center; }
    .info-item span:first-child { color: var(--soft); font-size: 10px; }
    .info-sep { width: 1px; height: 24px; background: var(--border); }

    /* credit */
    .credit {
      margin: 18px 26px 26px;
      border: 1px solid var(--border); border-radius: 14px;
      padding: 15px; background: var(--surface); position: relative; overflow: hidden;
    }
    .credit::before {
      content:''; position: absolute; top:0; left:0; right:0; height: 1px;
      background: linear-gradient(90deg, transparent, rgba(68,138,255,0.5), transparent);
    }
    .credit-top { display: flex; align-items: center; gap: 10px; margin-bottom: 12px; }
    .credit-av {
      width: 36px; height: 36px; border-radius: 50%;
      background: linear-gradient(135deg, var(--blue), #7c4dff);
      display: flex; align-items: center; justify-content: center;
      font-size: 15px; flex-shrink: 0; border: 1px solid rgba(68,138,255,0.3);
    }
    .credit-name { font-size: 13px; font-weight: 700; color: var(--text); letter-spacing: 0.3px; }
    .credit-role { font-family: 'Space Mono', monospace; font-size: 9px; color: var(--muted); margin-top: 2px; }
    .social-row { display: flex; gap: 8px; }
    .slink {
      flex: 1; display: inline-flex; align-items: center; justify-content: center;
      gap: 6px; padding: 8px; border-radius: 10px; text-decoration: none;
      font-size: 11px; font-weight: 700; transition: all 0.25s; border: 1px solid transparent;
    }
    .slink.gh { background: rgba(255,255,255,0.04); color: #c9d1d9; border-color: #30363d; }
    .slink.gh:hover { background: rgba(255,255,255,0.1); border-color: #8b949e; transform: translateY(-2px); }
    .slink.ig { background: rgba(225,48,108,0.08); color: #f48fb1; border-color: rgba(225,48,108,0.25); }
    .slink.ig:hover { background: rgba(225,48,108,0.18); border-color: rgba(225,48,108,0.6); transform: translateY(-2px); }

    /* toast */
    .toast {
      position: fixed; bottom: 28px; left: 50%;
      transform: translateX(-50%) translateY(10px);
      background: #1a2332; border: 1px solid var(--border);
      color: var(--text); padding: 11px 22px; border-radius: 100px;
      font-family: 'Space Mono', monospace; font-size: 11px;
      opacity: 0; transition: all 0.3s ease;
      pointer-events: none; white-space: nowrap;
      z-index: 200; box-shadow: 0 8px 30px rgba(0,0,0,0.5);
    }
    .toast.show { opacity: 1; transform: translateX(-50%) translateY(0); }

    /* busy overlay */
    .busy-overlay {
      display: none; position: fixed; inset: 0;
      background: rgba(8,12,20,0.75); backdrop-filter: blur(4px);
      align-items: center; justify-content: center;
      z-index: 99; flex-direction: column; gap: 16px;
    }
    .busy-overlay.active { display: flex; }
    .spinner {
      width: 40px; height: 40px;
      border: 3px solid var(--border); border-top-color: var(--green);
      border-radius: 50%; animation: spin 0.75s linear infinite;
    }
    .busy-txt { font-family: 'Space Mono', monospace; font-size: 11px; color: var(--soft); letter-spacing: 1px; }
  </style>
</head>
<body>

  <div class="busy-overlay" id="busyOv">
    <div class="spinner"></div>
    <div class="busy-txt" id="busyTxt">SERVO MOVING...</div>
  </div>

  <div class="toast" id="toast"></div>

  <div class="card">
    <div class="accent-bar"></div>

    <div class="header">
      <div>
        <h1>FAN CONTROLLER</h1>
        <p>ESP8266 &bull; DUAL SERVO &bull; WI-FI</p>
      </div>
      <div class="wifi-pill">
        <div class="wifi-dot"></div>LIVE
      </div>
    </div>

    <div class="fan-section">
      <div class="fan-outer off" id="fanOuter">
        <div class="fan-emoji">&#127744;</div>
      </div>
      <div class="status-wrap">
        <div class="status-badge off" id="badge">
          <div class="s-dot"></div>
          <span id="badgeTxt">FAN OFF</span>
        </div>
      </div>
    </div>

    <div class="servo-row">
      <div class="servo-chip" id="chip1">
        <div class="chip-head">
          <span class="chip-num">SERVO 1</span>
          <div class="chip-led"></div>
        </div>
        <div class="chip-title">ON Motor</div>
        <div class="chip-state" id="s1txt">Standby</div>
        <div class="chip-pin">D1 &bull; GPIO5</div>
      </div>
      <div class="servo-chip" id="chip2">
        <div class="chip-head">
          <span class="chip-num">SERVO 2</span>
          <div class="chip-led"></div>
        </div>
        <div class="chip-title">OFF Motor</div>
        <div class="chip-state" id="s2txt">Standby</div>
        <div class="chip-pin">D2 &bull; GPIO4</div>
      </div>
    </div>

    <div class="btn-section">
      <div class="btn-row">
        <button class="btn btn-on"  id="bOn"  onclick="cmd('on')">&#10003; Turn ON</button>
        <button class="btn btn-off" id="bOff" onclick="cmd('off')">&#10005; Turn OFF</button>
      </div>
      <div class="hint-row">
        <div class="hint">Servo 1 fires &rarr; D1</div>
        <div class="hint">Servo 2 fires &rarr; D2</div>
      </div>
    </div>

    <div class="info-strip">
      <div class="info-item"><span id="ipTxt">--</span><span>IP ADDRESS</span></div>
      <div class="info-sep"></div>
      <div class="info-item"><span id="syncTxt">--</span><span>LAST SYNC</span></div>
      <div class="info-sep"></div>
      <div class="info-item"><span id="cntTxt">0</span><span>COMMANDS</span></div>
    </div>

    <div class="credit">
      <div class="credit-top">
        <div class="credit-av">&#128736;</div>
        <div>
          <div class="credit-name">ILHAM FAROOQUE</div>
          <div class="credit-role">DEVELOPER &bull; ESP8266 PROJECT</div>
        </div>
      </div>
      <div class="social-row">
        <a class="slink gh" href="https://github.com/ilham786" target="_blank" rel="noopener">
          &#128121; GitHub
        </a>
        <a class="slink ig" href="https://www.instagram.com/ilham_farooque/" target="_blank" rel="noopener">
          &#128248; Instagram
        </a>
      </div>
    </div>

  </div>

<script>
  let fanOn = false, busy = false, cnt = 0;

  function poll() {
    fetch('/status').then(r=>r.json())
      .then(d=>{ fanOn=d.fan_on; draw(null); })
      .catch(()=>{});
  }

  function draw(active) {
    const fo = document.getElementById('fanOuter');
    const bg = document.getElementById('badge');
    const bt = document.getElementById('badgeTxt');
    const b1 = document.getElementById('bOn');
    const b2 = document.getElementById('bOff');
    const c1 = document.getElementById('chip1');
    const c2 = document.getElementById('chip2');
    const t1 = document.getElementById('s1txt');
    const t2 = document.getElementById('s2txt');
    if (fanOn) {
      fo.className='fan-outer on'; bg.className='status-badge on'; bt.textContent='FAN ON';
      b1.disabled=true; b2.disabled=false;
    } else {
      fo.className='fan-outer off'; bg.className='status-badge off'; bt.textContent='FAN OFF';
      b1.disabled=false; b2.disabled=true;
    }
    c1.className='servo-chip'; c2.className='servo-chip';
    t1.textContent='Standby'; t2.textContent='Standby';
    if (active==='on')  { c1.classList.add('fon');  t1.textContent='PRESSING...'; }
    if (active==='off') { c2.classList.add('foff'); t2.textContent='PRESSING...'; }
    const n=new Date();
    document.getElementById('syncTxt').textContent=n.toLocaleTimeString([],{hour:'2-digit',minute:'2-digit',second:'2-digit'});
  }

  function cmd(c) {
    if (busy) { toast('Please wait — servo moving'); return; }
    busy=true;
    document.getElementById('busyTxt').textContent = c==='on'?'SERVO 1 PRESSING ON...':'SERVO 2 PRESSING OFF...';
    document.getElementById('busyOv').classList.add('active');
    draw(c);
    fetch('/'+c).then(r=>r.json())
      .then(d=>{ fanOn=d.fan_on; cnt++; document.getElementById('cntTxt').textContent=cnt; draw(null); toast(fanOn?'Fan ON — Servo 1 fired':'Fan OFF — Servo 2 fired'); })
      .catch(()=>{ draw(null); toast('Connection error'); })
      .finally(()=>{ busy=false; document.getElementById('busyOv').classList.remove('active'); });
  }

  function toast(m) {
    const t=document.getElementById('toast');
    t.textContent=m; t.classList.add('show');
    setTimeout(()=>t.classList.remove('show'),2800);
  }

  fetch('/info').then(r=>r.json()).then(d=>{ document.getElementById('ipTxt').textContent=d.ip; });
  poll(); setInterval(poll,3000);
</script>
</body>
</html>
)rawhtml";

// ================================================================
//  SERVO FUNCTIONS
// ================================================================
void servo1_PressON() {
  Serial.print(F("  [S1] Pressing ON -> ")); Serial.print(S1_PRESS); Serial.println(F("deg"));
  servoON.write(S1_PRESS);
  delay(PRESS_HOLD_MS);
  servoON.write(S1_NEUTRAL);
  delay(RETURN_DELAY_MS);
  Serial.print(F("  [S1] Back to neutral -> ")); Serial.print(S1_NEUTRAL); Serial.println(F("deg"));
}

void servo2_PressOFF() {
  Serial.print(F("  [S2] Pressing OFF -> ")); Serial.print(S2_PRESS); Serial.println(F("deg"));
  servoOFF.write(S2_PRESS);
  delay(PRESS_HOLD_MS);
  servoOFF.write(S2_NEUTRAL);
  delay(RETURN_DELAY_MS);
  Serial.print(F("  [S2] Back to neutral -> ")); Serial.print(S2_NEUTRAL); Serial.println(F("deg"));
}

void turnFanON() {
  Serial.println(F("\n[CMD] FAN ON  -> Servo 1 / D1"));
  servo1_PressON();
  fanIsOn = true;
  Serial.println(F("[CMD] Done -> Fan state: ON\n"));
}

void turnFanOFF() {
  Serial.println(F("\n[CMD] FAN OFF -> Servo 2 / D2"));
  servo2_PressOFF();
  fanIsOn = false;
  Serial.println(F("[CMD] Done -> Fan state: OFF\n"));
}

// ================================================================
//  HTTP HELPERS
// ================================================================
String jsonState() {
  String s = "{\"fan_on\":";
  s += fanIsOn ? "true" : "false";
  s += ",\"busy\":";
  s += isBusy  ? "true" : "false";
  s += "}";
  return s;
}

void replyJSON(String body, int code = 200) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(code, "application/json", body);
}

// ================================================================
//  HTTP ROUTE HANDLERS
// ================================================================
void handleRoot()   { server.send_P(200, "text/html", INDEX_HTML); }
void handleStatus() { replyJSON(jsonState()); }
void handleInfo()   {
  String s = "{\"ip\":\"";
  s += WiFi.localIP().toString();
  s += "\"}";
  replyJSON(s);
}
void handleNotFound() { server.send(404, "text/plain", "Not Found"); }

void handleON() {
  if (!isBusy) {
    isBusy = true;
    if (!fanIsOn) turnFanON();
    else Serial.println(F("[SKIP] Fan already ON"));
    isBusy = false;
  }
  replyJSON(jsonState());
}

void handleOFF() {
  if (!isBusy) {
    isBusy = true;
    if (fanIsOn) turnFanOFF();
    else Serial.println(F("[SKIP] Fan already OFF"));
    isBusy = false;
  }
  replyJSON(jsonState());
}

// ================================================================
//  SETUP
// ================================================================
void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println(F("\n================================================"));
  Serial.println(F("  Wireless Fan Controller v2.0 - Dual Servo"));
  Serial.println(F("  Developer : ILHAM FAROOQUE"));
  Serial.println(F("  GitHub    : github.com/ilham786"));
  Serial.println(F("================================================\n"));

  // Attach Servo 1 (ON servo)
  servoON.attach(SERVO1_PIN);
  servoON.write(S1_NEUTRAL);
  delay(600);
  Serial.print(F("Servo 1 (ON)  ready  D1/GPIO5  neutral="));
  Serial.print(S1_NEUTRAL); Serial.println(F("deg"));

  // Attach Servo 2 (OFF servo)
  servoOFF.attach(SERVO2_PIN);
  servoOFF.write(S2_NEUTRAL);
  delay(600);
  Serial.print(F("Servo 2 (OFF) ready  D2/GPIO4  neutral="));
  Serial.print(S2_NEUTRAL); Serial.println(F("deg"));

  // Connect Wi-Fi
  Serial.print(F("\nConnecting to Wi-Fi ["));
  Serial.print(WIFI_SSID);
  Serial.print(F("] "));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(F("."));
    if (++tries > 40) {
      Serial.println(F("\nERROR: Wi-Fi failed. Check SSID/password and restart."));
      while (true) delay(1000);
    }
  }

  Serial.println();
  Serial.println(F("Wi-Fi connected!"));
  Serial.print(F("IP  : ")); Serial.println(WiFi.localIP());
  Serial.print(F("RSSI: ")); Serial.print(WiFi.RSSI()); Serial.println(F(" dBm"));

  // Register HTTP routes
  server.on("/",       handleRoot);
  server.on("/on",     handleON);
  server.on("/off",    handleOFF);
  server.on("/status", handleStatus);
  server.on("/info",   handleInfo);
  server.onNotFound(   handleNotFound);

  server.begin();
  Serial.println(F("\n================================================"));
  Serial.print(F("  http://")); Serial.print(WiFi.localIP()); Serial.println(F("/"));
  Serial.println(F("  /on  -> Servo 1 (D1) presses switch ON"));
  Serial.println(F("  /off -> Servo 2 (D2) presses switch OFF"));
  Serial.println(F("================================================\n"));
}

// ================================================================
//  LOOP
// ================================================================
void loop() {
  server.handleClient();
  yield();
}
