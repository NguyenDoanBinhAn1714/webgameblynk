#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "DOAN NHI_2.4G";
const char* password =  "nhichuc01";

WebServer server(80);
int ledPin = 2;  // LED GPIO 2

// Web game
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Mini Fight Game</title>
</head>
<body>
  <canvas id="game" width="400" height="300"></canvas>
  <script>
    const canvas=document.getElementById("game");
    const ctx=canvas.getContext("2d");

    let player={x:50,y:220,w:30,h:30,color:"blue",hp:3};
    let enemy ={x:300,y:220,w:30,h:30,color:"red",hp:3};

    document.addEventListener("keydown",e=>{
      if(e.key==="ArrowLeft") player.x -= 10;
      if(e.key==="ArrowRight") player.x += 10;
      if(e.key===" "){
        if(Math.abs(player.x-enemy.x)<40){
          enemy.hp--;
          if(enemy.hp<=0) endGame("win");
        }
      }
    });

    function update(){
      ctx.clearRect(0,0,400,300);
      ctx.fillStyle=player.color; ctx.fillRect(player.x,player.y,player.w,player.h);
      ctx.fillStyle=enemy.color;  ctx.fillRect(enemy.x,enemy.y,enemy.w,enemy.h);
      ctx.fillStyle="black"; ctx.fillText("Enemy HP: "+enemy.hp,300,20);
      requestAnimationFrame(update);
    }

    function endGame(r){
      alert(r=="win"?"Bạn thắng!":"Bạn thua!");
      fetch("/led?status="+r);
    }

    update();
  </script>
</body>
</html>
)rawliteral";

// ⚡ Biến để hẹn giờ tắt LED
unsigned long ledOnTime = 0; 
bool ledActive = false;

void handleLed(){
  if(server.hasArg("status")){
    String st = server.arg("status");
    if(st=="win"){
      digitalWrite(ledPin, HIGH);   // bật LED
      ledOnTime = millis();         // ghi lại thời điểm bật
      ledActive = true;
      server.send(200,"text/plain","LED WIN: ON (auto off)");
    } else if(st=="lose"){
      digitalWrite(ledPin, LOW);    // lose → LED tắt luôn
      ledActive = false;
      server.send(200,"text/plain","LED LOSE: OFF");
    } else {
      server.send(200,"text/plain","Unknown state");
    }
  } else {
    server.send(400,"text/plain","Missing status arg");
  }
}

void setup(){
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối WiFi!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", [](){ server.send_P(200,"text/html",index_html); });
  server.on("/led", handleLed);

  server.begin();
  Serial.println("WebServer đã sẵn sàng!");
}

void loop(){
  server.handleClient();

  // ⏳ Kiểm tra nếu LED đã bật quá 2 giây thì tắt
  if(ledActive && millis() - ledOnTime >= 2000){
    digitalWrite(ledPin, LOW);
    ledActive = false;
  }
}
