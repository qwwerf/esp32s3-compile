#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

WebServer server(80);
Preferences prefs;

String ssid;
String password;

// AP配网页面
void handleConfigPage() {
  String html = R"HTML(
<!DOCTYPE html>
<meta charset="UTF-8">
<title>ESP32‑S3配网</title>
<body style="text-align:center;margin-top:60px;font-size:22px;">
<h2>配置WiFi网络</h2>
<form method="POST" action="/save">
SSID:<br><input name="ssid" type="text"><br><br>
密码:<br><input name="pass" type="password"><br><br>
<button type="submit">保存并重启设备</button>
</form>
</body>
)HTML";
  server.send(200, "text/html", html);
}

// 保存WiFi参数
void handleSave() {
  if(server.hasArg("ssid") && server.hasArg("pass")){
    ssid = server.arg("ssid");
    password = server.arg("pass");
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    server.send(200,"text/html","<h2>保存成功，设备正在重启...</h2>");
    delay(1200);
    ESP.restart();
  }else{
    server.send(200,"text/html","<h2>参数不全，请返回重新填写</h2>");
  }
}

// 正常联网后的主页
void handleRoot() {
  String page = "<html>";
  page += "<head><meta charset='UTF-8'><title>ESP32‑S3微型网页服务器</title></head>";
  page += "<body style='text‑align:center;margin‑top:80px;font‑size:24px;'>";
  page += "<h1>✅ ESP32‑S3网页服务器运行正常</h1>";
  page += "<p>局域网访问成功</p>";
  page += "<p>设备IP："+WiFi.localIP().toString()+"</p>";
  page += "</body></html>";
  server.send(200, "text/html", page);
}

// 启动AP热点配网模式
void startAPMode(){
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32‑S3_配置热点");
  server.on("/", handleConfigPage);
  server.on("/save", handleSave);
  Serial.println("⚠️进入配网模式");
  Serial.println("手机连接WiFi: ESP32‑S3_配置热点");
  Serial.println("浏览器打开 192.168.4.1 设置WiFi");
}

void setup() {
  Serial.begin(115200);
  prefs.begin("wifi_config", false);

  // 读取保存的WiFi
  ssid = prefs.getString("ssid","");
  password = prefs.getString("pass","");

  // 如果没有保存WiFi，直接开启配网热点
  if(ssid.length() == 0){
    startAPMode();
  }else{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    int timeout = 0;
    while(WiFi.status() != WL_CONNECTED && timeout <25){
      delay(500);
      timeout++;
      Serial.print(".");
    }
    // 连WiFi失败，切配网模式
    if(WiFi.status() != WL_CONNECTED){
      startAPMode();
    }else{
      Serial.println("\n✅WiFi连接成功");
      Serial.print("设备IP：");
      Serial.println(WiFi.localIP());
      server.on("/", handleRoot);
    }
  }
  server.begin();
}

void loop() {
  server.handleClient();
}
