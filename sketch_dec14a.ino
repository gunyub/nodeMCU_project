#include <UniversalTelegramBot.h> // 텔레그램 봇과 통신을 위한 라이브러리
#include <WiFiClientSecure.h>     // HTTPS 보안 통신을 위한 라이브러리
#include <ESP8266WiFi.h>          // ESP8266의 Wi-Fi 연결 제어를 위한 라이브러리
#include <DHT.h>                  // 온습도 센서를 제어하기 위한 라이브러리

// Wi-Fi 설정
const char* ssid = "SK_EEBC_2.4G";       // Wi-Fi 이름(SSID)
const char* password = "BGC04@8086";     // Wi-Fi 비밀번호

// Telegram Bot 설정
const String botToken = "7943684933:AAH-Gl9th_1HWeDh2gb79yu-KBLzdRTGmtQ"; // 텔레그램 봇 인증 토큰
const String chatID = "5885492660";                                       // 메시지를 받을 텔레그램 채팅 ID

WiFiClientSecure secured_client;               // HTTPS 통신 클라이언트 객체 생성
UniversalTelegramBot bot(botToken, secured_client); // 텔레그램 봇 객체 생성

// 조도 센서 설정
#define LDR_PIN A0           // 조도 센서(LDR)가 연결된 핀 (NodeMCU의 A0 핀)
int LDR_THRESHOLD = 30;      // 조도 센서의 밝기 임계값 (이 값을 초과하면 문이 열렸다고 판단)

// DHT 센서 설정
#define DHTPIN D1            // DHT 센서 데이터 핀
#define DHTTYPE DHT11        // 사용된 DHT 센서의 유형 (DHT11)
DHT dht(DHTPIN, DHTTYPE);    // DHT 센서 객체 생성

void setup() {
  Serial.begin(115200);       // 시리얼 통신 초기화 (디버깅 출력을 위해 사용)

  // Wi-Fi 연결 설정
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password); // Wi-Fi 연결 시작
  while (WiFi.status() != WL_CONNECTED) { // Wi-Fi가 연결될 때까지 대기
    delay(500);
    Serial.print(".");       // 연결 대기 중 '.' 출력
  }
  Serial.println("\nWi-Fi connected"); // Wi-Fi 연결 성공 메시지 출력
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());      // 연결된 기기의 IP 주소 출력

  // UniversalTelegramBot TLS 설정
  configTime(0, 0, "pool.ntp.org");    // TLS 인증서를 위해 시간 동기화
  secured_client.setInsecure();        // 인증서 검증을 비활성화하여 통신 가능하도록 설정

  dht.begin(); // DHT 센서 초기화
}

void loop() {
  // LDR 값 읽기
  int ldrValue = analogRead(LDR_PIN); // 조도 센서(LDR) 값 읽기
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);          // 읽어온 조도 센서 값을 시리얼 출력

  // DHT 값 읽기
  float temperature = dht.readTemperature(); // DHT 센서에서 온도 값 읽기
  float humidity = dht.readHumidity();       // DHT 센서에서 습도 값 읽기

  // 온습도 센서 읽기 실패 시 처리
  if (isnan(temperature) || isnan(humidity)) { // 데이터가 유효하지 않을 경우
    Serial.println("Failed to read from DHT sensor!"); // 오류 메시지 출력
    return; // 루프 종료
  }

  // 이상 상태 플래그 초기화 및 알림 메시지 생성
  bool isIssue = false;                   // 이상 상태 플래그 (초기화: false)
  String message = "리퍼 컨테이너 상태 알림\n\n"; // 기본 알림 메시지 텍스트

  // 빛 감지 상태 확인
  if (ldrValue > LDR_THRESHOLD) {         // 조도 값이 임계값을 초과하면
    message += "문 상태: 열림 (빛 감지됨)\n"; // 문이 열렸다고 판단
    isIssue = true;                       // 이상 상태 플래그 설정
  } else {
    message += "문 상태: 닫힘\n";         // 문이 닫혔다고 판단
  }

  // 온도 상태 확인
  if (temperature < 23 || temperature > 30) { // 온도가 정상 범위를 벗어난 경우
    message += "온도 이상: " + String(temperature) + " °C\n"; // 온도 이상 메시지 추가
    isIssue = true;                          // 이상 상태 플래그 설정
  } else {
    message += "온도: " + String(temperature) + " °C\n"; // 정상 온도 메시지 추가
  }

  // 습도 상태 확인
  if (humidity < 5 || humidity > 20) {       // 습도가 정상 범위를 벗어난 경우
    message += "습도 이상: " + String(humidity) + " %\n"; // 습도 이상 메시지 추가
    isIssue = true;                          // 이상 상태 플래그 설정
  } else {
    message += "습도: " + String(humidity) + " %\n"; // 정상 습도 메시지 추가
  }

  // 이상 상태가 있을 경우 텔레그램 메시지 전송
  if (isIssue) { // 이상 상태가 감지되면
    bool success = bot.sendMessage(chatID, message, ""); // 텔레그램 메시지 전송
  }

  delay(1000); // 1초 대기 후 루프 반복
}
