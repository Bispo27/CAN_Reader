#include <WiFi.h>
#include <ESPAsyncWebSrv.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <ESP32TimerInterrupt.h>

const char* ssid = "SeuSSID";
const char* password = "SuaSenha";
uint8_t canRegister[12];
uint8_t byteCAN = 0;
uint8_t indexCanRegister = 0;
typedef struct no{
  uint32_t id;
  uint8_t data[8];
  uint8_t dlc;
}CANFrame;
String pageCANHTML = "<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"<title>Wi-Fi CAN Reader</title>\n"
"<style>\n"
".container {\n"
"  display: flex;\n"
"  justify-content: flex-start;\n"
"  align-items: center;\n"
"  height: 100vh;\n"
"  flex-direction: column;\n"
"}\n"
"\n"
".elementos {\n"
"  display: flex;\n"
"  align-items: center;\n"
"}\n"
"\n"
".janela {\n"
"  width: 400px;\n"
"  height: 200px;\n"
"  background-color: #f0f0f0;\n"
"  border: 1px solid #ccc;\n"
"  text-align: center;\n"
"  padding: 10px;\n"
"  overflow-y: scroll;\n"
"  margin-top: 10px;\n"
"}\n"
"\n"
".select {\n"
"  width: 150px;\n"
"  margin-right: 10px;\n"
"  text-align: center;\n"
"}\n"
"\n"
".button {\n"
"  margin-right: 10px;\n"
"}\n"
"\n"
".valor-selecionado {\n"
"  width: 150px;\n"
"  border: 1px solid #ccc;\n"
"  padding: 10px;\n"
"  margin-right: 5px;\n"
"  text-align: center;\n"
"  background-color: #ffffff;\n"
"}\n"
"\n"
".buttons {\n"
"  margin-right: 25px;\n"
"}\n"
"</style>\n"
"</head>\n"
"<body>\n"
"<div class=\"container\">\n"
"  <h1>Wi-Fi CAN Reader</h1>\n"
"  <div class=\"elementos\">\n"
"    <select id=\"selecao\">\n"
"      <option value='125'>125 kbps</option>\n"
"      <option value='250'>250 kbps</option>\n"
"      <option value='500'>500 kbps</option>\n"
"      <option value='800'>800 kbps</option>\n"
"      <option value='1000'>1000 kbps</option>\n"
"    </select>\n"
"    <button onclick=\"enviarSelecao()\">Select</button>\n"
"    <div class=\"valor-selecionado\" id=\"valor-selecionado\"></div>\n"
"  </div>\n"
"  <div class=\"janela\" id=\"valores\"></div>\n"
"  <div class=\"elementos\">\n"
"    <button id=\"startStopButton\" onclick=\"toggleStartStop()\">START</button>\n"
"    <button id=\"downloadButton\" onclick=\"downloadLog()\">DOWNLOAD</button>\n"
"    <button onclick=\"limparJanela()\">Limpar Janela</button>\n"
"    <input type=\"checkbox\" id=\"autoScrollCheckbox\" onchange=\"toggleAutoScroll()\"> Auto Scroll\n"
"    <div id=\"log\"></div>\n"
"  </div>\n"
"</div>\n"
"\n"
"<script>\n"
"function enviarSelecao() {\n"
"  var selecao = document.getElementById('selecao').value;\n"
"  var xhttp = new XMLHttpRequest();\n"
"  document.getElementById('valor-selecionado').innerHTML = selecao + ' kbps';\n"
"  xhttp.onreadystatechange = function() {\n"
"    if (this.readyState == 4 && this.status == 200) {\n"
"      document.getElementById('valores').innerHTML = this.responseText;\n"
"      document.getElementById('valor-selecionado').innerHTML = selecao + ' kbps';\n"
"    }\n"
"  };\n"
"  xhttp.open('POST', '/select', true);\n"
"xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');"
"  xhttp.send('bitrate='+selecao);\n"
"}\n"
"\n"
"var autoScroll = true;\n"
"\n"
"function toggleAutoScroll() {\n"
"  var checkbox = document.getElementById(\"autoScrollCheckbox\");\n"
"  var valoresDiv = document.getElementById('valores');\n"
"  valoresDiv.style.overflowY = checkbox.checked ? \"scroll\" : \"auto\";\n"
"  autoScroll = checkbox.checked;\n"
"}\n"
"document.getElementById('autoScrollCheckbox').addEventListener('change', toggleAutoScroll);\n"
"\n"
"function checkScrollPosition() {\n"
"  var valoresDiv = document.getElementById('valores');\n"
"  if (valoresDiv.scrollTop + valoresDiv.clientHeight === valoresDiv.scrollHeight) {\n"
"    autoScroll = true;\n"
"  } else {\n"
"    autoScroll = false;\n"
"  }\n"
"}\n"
"\n"
"function scrollToBottom() {\n"
"  if (autoScroll) {\n"
"    var valoresDiv = document.getElementById('valores');\n"
"    valoresDiv.scrollTop = valoresDiv.scrollHeight;\n"
"  }\n"
"}\n"
"\n"
"document.getElementById('valores').addEventListener('scroll', checkScrollPosition);\n"
"\n"
"function limparJanela() {\n"
"  document.getElementById('valores').innerHTML = \"\";\n"
"}\n"
"\n"
"function appendLog(data) {\n"
"  logData.push(data);\n"
"  var logElement = document.getElementById(\"log\");\n"
"  var valoresDiv = document.getElementById('valores');\n"
"  var text = valoresDiv.innerHTML += data;\n"
"  valoresDiv.innerHTML = text.replace(/\\r\\n|\\n|\\r/g, \"<br>\");\n"
"  scrollToBottom();\n"
"}\n"
"\n"
"var intervalID;\n"
"\n"
"function startInterval() {\n"
"  intervalID = setInterval(function() {\n"
"    var xhttp = new XMLHttpRequest();\n"
"    xhttp.open('GET', \"/can-read\", true);\n"
"    xhttp.send();\n"
"    xhttp.onload = function() {\n"
"      appendLog(xhttp.responseText);\n"
"    };\n"
"  }, 100);\n"
"}\n"
"\n"
"function stopInterval() {\n"
"  clearInterval(intervalID);\n"
"}\n"
"\n"
"function downloadLog() {\n"
"  var logText = logData.join(\",\");\n"
"  var blob = new Blob([logText], { type: \"text/plain\" });\n"
"  var url = window.URL.createObjectURL(blob);\n"
"  var a = document.createElement(\"a\");\n"
"  a.style.display = \"none\";\n"
"  a.href = url;\n"
"  a.download = \"logsCAN.log\";\n"
"  document.body.appendChild(a);\n"
"  a.click();\n"
"  window.URL.revokeObjectURL(url);\n"
"  logData = [];\n"
"  document.getElementById(\"log\").innerHTML = \"\";\n"
"}\n"
"\n"
"var isStarted = false;\n"
"var logData = [];\n"
"\n"
"function toggleStartStop() {\n"
"  isStarted = !isStarted;\n"
"  var xhttp = new XMLHttpRequest();\n"
"  var startStopButton = document.getElementById(\"startStopButton\");\n"
"  xhttp.onreadystatechange = function() {\n"
"    if (this.readyState == 4 && this.status == 200) {\n"
"      if (isStarted) {\n"
"        startStopButton.innerHTML = \"STOP\";\n"
"        startInterval();\n"
"      } else {\n"
"        startStopButton.innerHTML = \"START\";\n"
"        stopInterval();\n"
"      }\n"
"    }\n"
"  };\n"
"  if (isStarted) {\n"
"    startStopButton.innerHTML = \"STOP\";\n"
"    xhttp.open('POST', '/start', true);\n"
"    xhttp.send();\n"
"  } else {\n"
"    startStopButton.innerHTML = \"START\";\n"
"    xhttp.open('POST', '/stop', true);\n"
"    xhttp.send();\n"
"    stopInterval();\n"
"  }\n"
"}\n"
"</script>\n"
"</body>\n"
"</html>";


AsyncWebServer server(80);
bool isSend = false;
String canMsg = "";
uint32_t bitRate = 125;
unsigned long periodo = 0;
SemaphoreHandle_t shared_var_mutex = NULL;

const int PINO_BIT = 2;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile bool leituraAtiva = false;
void IRAM_ATTR onSOF() {
  portENTER_CRITICAL_ISR(&mux);
  detachInterrupt(digitalPinToInterrupt(PINO_BIT));
  leituraAtiva = true;
  portEXIT_CRITICAL_ISR(&mux);
}

bool IRAM_ATTR onBitChange(void *arg) {
  if(leituraAtiva){
    portENTER_CRITICAL_ISR(&mux);
    canProcess(digitalRead(PINO_BIT));
    portEXIT_CRITICAL_ISR(&mux);
    return true;
  }
  return false;
}



const int BUFFER_SIZE = 10; // Tamanho do buffer (número máximo de frames)
CANFrame buffer[BUFFER_SIZE]; // Array para armazenar os frames
int head = 0; // Índice de inserção
int tail = 0; // Índice de remoção



// Função para adicionar um frame ao buffer
void adicionarFrame(const CANFrame& frame) {
  buffer[head] = frame;
  head = (head + 1) % BUFFER_SIZE; // Atualiza o índice de inserção (circular)
}

// Função para obter um frame do buffer
bool obterFrame(CANFrame& frame) {
  if (head != tail) {
    frame = buffer[tail];
    tail = (tail + 1) % BUFFER_SIZE; // Atualiza o índice de remoção (circular)
    return true;
  }
  return false; // Retorna falso se o buffer estiver vazio
}

void canProcess(bool bit){
  
  byteCAN <<= 1;
  byteCAN |= (bit ? 1 : 0);
  
  if (indexCanRegister < 12) {
    memmove(&canRegister[1], &canRegister[0], 11); // Desloca os bytes no vetor
    canRegister[0] = byteCAN; // Coloca o novo byte no início
    indexCanRegister++;
  }else {
    CANFrame frame;
    frame.id = canRegister[0];
    memcpy(frame.data, &canRegister[1], 8);
    frame.dlc=8;
    adicionarFrame(frame);
    indexCanRegister = 0;
    leituraAtiva = false;
  }
}

void controlSendCAN(void *pvParameter){
  uint64_t var = 0;
  timeval tv;
  tv.tv_sec = 1694738054;
  settimeofday(&tv, NULL);
  
  while(1){
    if(isSend){
        CANFrame frame;
        if (obterFrame(frame)) {
          // Você tem um frame válido em 'frame', faça o que precisar com ele
          if(xSemaphoreTake(shared_var_mutex, portMAX_DELAY) == pdTRUE){
            canMsg +=formatarFrame(frame, String("can0")); // Por exemplo, formate o frame
            Serial.print(canMsg);
            xSemaphoreGive(shared_var_mutex);//libera
          }
        }
    }
    var++;
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}


String formatarFrame(const CANFrame& frame, const String& interface) {
  char buffer[64];
  // Obter carimbo de data/hora
  unsigned long currentTime = millis();
  unsigned long seconds = currentTime / 1000;
  unsigned long millisecs = currentTime % 1000;

  // Formatar a string diretamente no buffer
  sprintf(buffer, "(%lu.%03lu) %s %lX [%d] %02X %02X %02X %02X %02X %02X %02X %02X",
          seconds, millisecs, interface.c_str(), frame.id, frame.dlc,
          frame.data[0], frame.data[1], frame.data[2], frame.data[3],
          frame.data[4], frame.data[5], frame.data[6], frame.data[7]);

  // Imprimir ou enviar a string conforme necessário
  Serial.println(buffer); // Exemplo: (1586415101.652641) can0 123 [4] 01 02 03 04
  return buffer;
}

void setup() {
  Serial.begin(115200);
  delay(100);
  memset(canRegister, 0, 12 * sizeof(uint8_t));
  WiFi.mode(WIFI_AP);
  IPAddress ip(3, 3, 3, 3);
  WiFi.softAPConfig(ip, IPAddress(3, 3, 3, 3), IPAddress(255, 255, 255, 0));
  delay(100);
  WiFi.softAP(ssid, password);
  pinMode(PINO_BIT, INPUT);
  ESP32Timer ITimer(0);

  Serial.println("IP do Ponto de Acesso (ESP32): " + WiFi.softAPIP().toString());

  
  shared_var_mutex = xSemaphoreCreateMutex();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200,"text/html", pageCANHTML);
  });
  server.on("/start", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200);
    isSend = true;
  });
  server.on("/stop", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200);
    isSend = false;
  });
  server.on("/can-read", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200,"text/plain", canMsg);
    if(xSemaphoreTake(shared_var_mutex, portMAX_DELAY) == pdTRUE){
         request->send(200,"text/plain", canMsg);
         canMsg = "";
         xSemaphoreGive(shared_var_mutex);//libera
    }
  });
  server.on("/select", HTTP_POST, [&ITimer](AsyncWebServerRequest *request){
    String bitRateValue = request->arg("bitrate");
    bitRate = bitRateValue.toInt() * 1000.00;
    request->send(200);
    periodo = 1000000.00 / (bitRate * 1000.00);
    ITimer.attachInterruptInterval(periodo, onBitChange);
    Serial.println(bitRateValue);
    Serial.print("BitRate configurado: ");Serial.println(bitRate);
    Serial.print("Periodo configurado: ");Serial.println(periodo, 10);
  });
  server.begin();

  
  ITimer.attachInterruptInterval(periodo, onBitChange);
  attachInterrupt(digitalPinToInterrupt(PINO_BIT), onSOF, FALLING);
  
  xTaskCreatePinnedToCore(controlSendCAN, "controlSendCAN", 10240, NULL,
    0, NULL, 1);
}


void loop() {

  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
