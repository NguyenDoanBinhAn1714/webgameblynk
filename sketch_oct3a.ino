#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

const char* ssid = "DOAN NHI_2.4G";
const char* password =  "nhichuc01";

WebServer server(80);
WebSocketsServer webSocket(81);

const int ledPin = 2; // LED báo hiệu

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String msg = String((char*)payload);

    if (msg == "WIN") {
      Serial.println("Kết quả: WIN");
      digitalWrite(ledPin, HIGH);
      delay(2000);
      digitalWrite(ledPin, LOW);
    }
    else if (msg == "LOSE") {
      Serial.println("Kết quả: LOSE");
      digitalWrite(ledPin, HIGH);
      delay(5000);
      digitalWrite(ledPin, LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối WiFi!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Game Đối Kháng ESP32</title>
  <style>
    canvas { background: #eee; display:block; margin: 20px auto; }
    h2 { text-align:center; }
  </style>
</head>
<body>
  <h2>Game Đối Kháng ESP32</h2>
  <canvas id="gameCanvas" width="400" height="300"></canvas>
  <script>
    var ws = new WebSocket('ws://' + window.location.hostname + ':81/');
    let canvas = document.getElementById("gameCanvas");
    let ctx = canvas.getContext("2d");

    // Nhân vật chính
    let player = {x:50, y:200, w:30, h:30, color:"blue", hp:100};
    // Quái
    let enemy = {x:300, y:200, w:30, h:30, color:"red", hp:100};

    let keys = {};

    document.addEventListener("keydown", e => keys[e.key] = true);
    document.addEventListener("keyup", e => keys[e.key] = false);

    function update() {
      // Điều khiển nhân vật (WASD)
      if (keys["a"]) player.x -= 2;
      if (keys["d"]) player.x += 2;
      if (keys["w"]) player.y -= 2;
      if (keys["s"]) player.y += 2;

      // Tấn công (Space)
      if (keys[" "]) {
        if (Math.abs(player.x - enemy.x) < 35 && Math.abs(player.y - enemy.y) < 35) {
          enemy.hp -= 1;
        }
      }

      // Enemy "phản công" đơn giản
      if (Math.random() < 0.01) {
        if (Math.abs(player.x - enemy.x) < 35 && Math.abs(player.y - enemy.y) < 35) {
          player.hp -= 1;
        }
      }

      // Kết thúc game
      if (enemy.hp <= 0) {
        ws.send("WIN");
        alert("Bạn thắng!");
        resetGame();
      }
      if (player.hp <= 0) {
        ws.send("LOSE");
        alert("Bạn thua!");
        resetGame();
      }
    }

    function draw() {
      ctx.clearRect(0,0,canvas.width,canvas.height);
      // Player
      ctx.fillStyle = player.color;
      ctx.fillRect(player.x, player.y, player.w, player.h);
      ctx.fillStyle = "black";
      ctx.fillText("HP:"+player.hp, player.x, player.y-5);

      // Enemy
      ctx.fillStyle = enemy.color;
      ctx.fillRect(enemy.x, enemy.y, enemy.w, enemy.h);
      ctx.fillStyle = "black";
      ctx.fillText("HP:"+enemy.hp, enemy.x, enemy.y-5);
    }

    function loop() {
      update();
      draw();
      requestAnimationFrame(loop);
    }

    function resetGame() {
      player.x=50; player.y=200; player.hp=100;
      enemy.x=300; enemy.y=200; enemy.hp=100;
    }

    loop();
  </script>
</body>
</html>
    )rawliteral");
  });

  server.begin();
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}
