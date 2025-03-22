#include <Arduino.h>
#include <TaskScheduler.h>
#include <PinChangeInterrupt.h>

// LED 핀 정의
const int redPin = 11;    // 빨간불 LED 핀
const int yellowPin = 9;  // 노란불 LED 핀
const int greenPin = 10;  // 초록불 LED 핀

// 버튼 핀 정의
const int buttonPin2 = 2; // 버튼 2 핀 (긴급 모드)
const int buttonPin3 = 3; // 버튼 3 핀 (모든 LED 깜빡임 모드)
const int buttonPin4 = 4; // 버튼 4 핀 (시스템 ON/OFF)

// 가변저항 핀 정의
const int potPin = A0;    // 가변저항 핀
int previousPotValue = -1; // 이전 가변저항 값 저장 변수

// TaskScheduler 객체 생성
Scheduler runner;

//함수선언
bool redTaskOnEnable();
void redTaskOnDisable();
bool yellowTaskOnEnable();
void yellowTaskOnDisable();
bool greenTaskOnEnable();
void greenTaskOnDisable();
bool greenBlinkTaskOnEnable();
void greenBlinkTaskCallback();
void greenBlinkTaskOnDisable();
bool yellowAfterGreenBlinkTaskOnEnable();
void yellowAfterGreenBlinkTaskOnDisable();
void redBlinkTaskCallback();
void allLedsBlinkTaskCallback();
void readPotentiometerTaskCallback();
void SchAddTask();
void SchAllDeleteTask();
void AllLedsOff();

// Task 선언
Task redTask(2000, TASK_ONCE, NULL, &runner, false, redTaskOnEnable, redTaskOnDisable); // 빨간불 Task
Task yellowTask(500, TASK_ONCE, NULL, &runner, false, yellowTaskOnEnable, yellowTaskOnDisable); // 노란불 Task
Task greenTask(2000, TASK_ONCE, NULL, &runner, false, greenTaskOnEnable, greenTaskOnDisable); // 초록불 Task
Task greenBlinkTask(300, 6, greenBlinkTaskCallback, &runner, false, greenBlinkTaskOnEnable, greenBlinkTaskOnDisable); // 초록불 깜빡임 Task
Task yellowAfterGreenBlinkTask(500, TASK_ONCE, NULL, &runner, false, yellowAfterGreenBlinkTaskOnEnable, yellowAfterGreenBlinkTaskOnDisable); // 초록불 깜빡임 후 노란불 Task
Task redBlinkTask(500, TASK_FOREVER, redBlinkTaskCallback, &runner, false); // 빨간불 깜빡임 Task
Task allLedsBlinkTask(500, TASK_FOREVER, allLedsBlinkTaskCallback, &runner, false); // 모든 LED 깜빡임 Task
Task readPotentiometerTask(200, TASK_FOREVER, readPotentiometerTaskCallback, &runner, true); // 가변저항 값 읽기 Task

// 상태 변수
volatile bool button2Pressed = false; // 버튼 2 상태
volatile bool button3Pressed = false; // 버튼 3 상태
volatile bool button4Pressed = false; // 버튼 4 상태
volatile bool systemOn = true;        // 시스템 ON/OFF 상태
int potValue = 0;                     // 가변저항 값

// 신호등 모드 정의
enum Mode { TRAFFIC_LIGHT, EMERGENCY, BLINKING, SYSTEM_OFF };
Mode currentMode = TRAFFIC_LIGHT; // 현재 모드

// 신호등 지속 시간 변수
int redDuration = 2000;    // 빨간불 지속 시간 (기본값: 2초)
int yellowDuration = 500; // 노란불 지속 시간 (기본값: 0.5초)
int greenDuration = 2000;  // 초록불 지속 시간 (기본값: 2초)

// 버튼 2 인터럽트 핸들러 (긴급 모드)
void handleButton2Press() {
  if (digitalRead(buttonPin2) == LOW) {
/*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
    if (currentMode!=EMERGENCY) { // 현재 모드가 긴급 모드가 아닌 경우에만 실행
      currentMode = EMERGENCY; // 긴급 모드로 전환
    button2Pressed = true;  // 긴급모드일 때 버튼 상태  설정
    button3Pressed = false; // 블링킹 모드와 충돌 방지를 위해 버튼 상태 설정
    systemOn = true; // 시스템 상태 설정
/*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
      Serial.print("Mode: Emergency\n");
      SchAllDeleteTask(); // 모든 Task 삭제
      AllLedsOff(); // 모든 LED 끄기
      Serial.print("RL\nYL\nGL\n"); // 모든 LED 꺼짐
      runner.addTask(redBlinkTask); // 빨간불 깜빡임 Task 추가
      redBlinkTask.enable(); // 빨간불 깜빡임 Task 활성화
      
    } else {
      currentMode = TRAFFIC_LIGHT; // 신호등 모드로 전환
      /*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
      button2Pressed = false;  // 긴급모드드버튼 상태  설정
      button3Pressed = false; // 블링킹 모드버튼 상태 설정
      systemOn = true; // 시스템 상태 설정
      /*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
      Serial.print("Mode: Traffic Light\n");
      redBlinkTask.disable(); // 빨간불 깜빡임 Task 비활성화
      AllLedsOff(); // 모든 LED 끄기
      Serial.print("RL\n"); // 빨간간 LED 꺼짐
      SchAddTask(); // 신호등 Task 추가
    }
  }
}

// 버튼 3 인터럽트 핸들러 (모든 LED 깜빡임 모드)
void handleButton3Press() {
  if (digitalRead(buttonPin3) == LOW) {
/*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
    if (currentMode!=BLINKING) { // 현재 모드가 모든 LED 깜빡임 모드가 아닌 경우에만 실행
      currentMode = BLINKING; // 모든 LED 깜빡임 모드로 전환
      button2Pressed = false;  // 긴급모드일 때 버튼 상태  설정
      button3Pressed = true; // 블링킹 모드와 충돌 방지를 위해 버튼 상태 설정
      systemOn = true; // 시스템 상태 설정
/*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
      Serial.print("Mode: Blinking\n");
      SchAllDeleteTask(); // 모든 Task 삭제
      runner.addTask(allLedsBlinkTask); // 모든 LED 깜빡임 Task 추가
      allLedsBlinkTask.enable(); // Task 활성화
    } else {
      currentMode = TRAFFIC_LIGHT; // 신호등 모드로 전환
      /*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
      button2Pressed = false;  // 긴급모드일 때 버튼 상태  설정
      button3Pressed = false; // 블링킹 모드와 충돌 방지를 위해 버튼 상태 설정
      systemOn = true; // 시스템 상태 설정
      /*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
      Serial.print("Mode: Traffic Light\n");
      allLedsBlinkTask.disable(); // 모든 LED 깜빡임 Task 비활성화
      AllLedsOff(); // 모든 LED 끄기
      Serial.print("RL\nYL\nGL\n"); // 모든 LED 꺼짐
      SchAddTask(); // 신호등 Task 추가
    }
  }
}

// 버튼 4 인터럽트 핸들러 (시스템 ON/OFF)
void handleButton4Press() {
  Serial.println("Button 4 Pressed");
  if (digitalRead(buttonPin4) == LOW) {
/*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
    if (currentMode==SYSTEM_OFF) { // 현재 모드가 시스템 OFF 모드가 아닌 경우에만 실행
      currentMode = TRAFFIC_LIGHT; // 신호등 모드로 전환
      button2Pressed = false;  // 긴급모드일 때 버튼 상태  설정
      button3Pressed = false; // 블링킹 모드와 충돌 방지를 위해 버튼 상태 설정
      systemOn = true; // 시스템 상태 설정
/*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
      Serial.print("Mode: Traffic Light\n");
      SchAddTask(); // 신호등 Task 추가

    } else {
      currentMode = SYSTEM_OFF; // 시스템 OFF 모드로 전환
      /*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
      button2Pressed = false;  // 긴급모드일 때 버튼 상태  설정
      button3Pressed = false; // 블링킹 모드와 충돌 방지를 위해 버튼 상태 설정
      systemOn = false; // 시스템 상태 설정
      /*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
      SchAllDeleteTask(); // 모든 Task 삭제
      AllLedsOff(); // 모든 LED 끄기
      Serial.print("Mode: System Off\n");
    }
  }
}



// 초기 설정 함수
void setup() {
  Serial.begin(9600); // 시리얼 통신 초기화

  // LED 핀 출력 모드 설정
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  // 버튼 핀 입력 모드 설정
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(buttonPin4, INPUT_PULLUP);

  // 가변저항 핀 입력 모드 설정
  pinMode(potPin, INPUT);

  // 버튼 인터럽트 설정
  attachPCINT(digitalPinToPCINT(buttonPin2), handleButton2Press, CHANGE);
  attachPCINT(digitalPinToPCINT(buttonPin3), handleButton3Press, CHANGE);
  attachPCINT(digitalPinToPCINT(buttonPin4), handleButton4Press, FALLING);

  // 초기 Task 시작
  redTask.restartDelayed();
}

// 시리얼 입력 처리 함수 (신호등 지속 시간 조절)
void processSerialInput() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // 시리얼 데이터 읽기
    input.trim(); // 공백 제거

    // 빨간불 지속 시간 설정
    if (input.startsWith("red:")) {
      redDuration = input.substring(4).toInt();
      redTask.setInterval(redDuration);
      Serial.println("Red duration updated: " + String(redDuration) + "ms");
    }
    // 노란불 지속 시간 설정
    else if (input.startsWith("yellow:")) {
      yellowDuration = input.substring(7).toInt();
      yellowTask.setInterval(yellowDuration);
      yellowAfterGreenBlinkTask.setInterval(yellowDuration);
      Serial.println("Yellow duration updated: " + String(yellowDuration) + "ms");
    }
    // 초록불 지속 시간 설정
    else if (input.startsWith("green:")) {
      greenDuration = input.substring(6).toInt();
      greenTask.setInterval(greenDuration);
      Serial.println("Green duration updated: " + String(greenDuration) + "ms");
    }

/*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
//p5.js에서 받은 input값에 따라 모드 변경
// 모드 제어: 신호등 모드로 전환
else if (input == "Mode: Traffic Light") {
  if(currentMode == TRAFFIC_LIGHT) return; // 이미 신호등 모드인 경우 아무 작업도 하지 않음
  currentMode = TRAFFIC_LIGHT; // 현재 모드를 신호등 모드로 설정
  button2Pressed = false;  // 긴급모드일 때 버튼 상태 설정
  button3Pressed = false; // 블링킹 모드 일때 버튼상태태 설정
  systemOn = true; // 시스템 ON 상태로 설정
  redBlinkTask.disable(); // 빨간불 깜빡임 Task 비활성화
  allLedsBlinkTask.disable(); // 모든 LED 깜빡임 Task 비활성화
  SchAllDeleteTask(); // 모든 Task 삭제
  AllLedsOff(); // 모든 LED 끄기
  SchAddTask(); // 신호등 Task 추가
} 
// 모드 제어: 긴급 모드로 전환
else if (input == "Mode: Emergency Mode") {
  if(currentMode == EMERGENCY) return; // 이미 긴급 모드인 경우 아무 작업도 하지 않음
  currentMode = EMERGENCY; // 현재 모드를 긴급 모드로 설정
  button2Pressed = true;  // 긴급모드일 때 버튼 상태도 true로 설정
  button3Pressed = false; // 블링킹 모드와 충돌 방지를 위해 버튼 상태 false로 설정
  systemOn = true; // 시스템 ON 상태로 설정
  SchAllDeleteTask(); // 모든 Task 삭제
  AllLedsOff(); // 모든 LED 끄기
  runner.addTask(redBlinkTask); // 빨간불 깜빡임 Task 추가
  redBlinkTask.enable(); // 빨간불 깜빡임 Task 활성화
} 
// 모드 제어: 모든 LED 깜빡임 모드로 전환
else if (input == "Mode: Blinking Mode") {
  if(currentMode == BLINKING) return; // 이미 모든 LED 깜빡임 모드인 경우 아무 작업도 하지 
  currentMode = BLINKING; // 현재 모드를 모든 LED 깜빡임 모드로 설정
  button2Pressed = false;  // 긴급모드일 때 버튼 상태 설정
  button3Pressed = true; // 블링킹 모드버튼 상태 설정
  systemOn = true; // 시스템 ON 상태로 설정
  SchAllDeleteTask(); // 모든 Task 삭제
  AllLedsOff(); // 모든 LED 끄기
  runner.addTask(allLedsBlinkTask); // 모든 LED 깜빡임 Task 추가
  allLedsBlinkTask.enable(); // 모든 LED 깜빡임 Task 활성화
} 
// 모드 제어: 시스템 OFF 모드로 전환
else if (input == "Mode: System Off") {
  if(currentMode == SYSTEM_OFF) return; // 이미 시스템 OFF 모드인 경우 아무 작업도 하지 않음
  currentMode = SYSTEM_OFF; // 현재 모드를 시스템 OFF로 설정
  button2Pressed = false;  // 긴급모드 설정
  button3Pressed = false; // 블링킹 모드 설정
  systemOn = false; // 시스템 설정
  SchAllDeleteTask(); // 모든 Task 삭제
  AllLedsOff(); // 모든 LED 끄기
}

/*************************W3 gesture 코드 추가한 부분(나머지는 1주차 과제와 동일) ***********************************/
      }
  }


// 메인 루프
void loop() {
  processSerialInput();  // 시리얼 입력 처리
  runner.execute();      // TaskScheduler 실행
  potValue = analogRead(potPin); // 가변저항 값 읽기
}

// 빨간불 Task가 활성화될 때 실행되는 함수
bool redTaskOnEnable() {
  if (!button2Pressed && !button3Pressed) { // 버튼 2와 3이 눌리지 않은 경우에만 실행
    Serial.println("Red Task Enabled");
    Serial.print("R\n"); // 빨간불 켜짐
    currentMode = TRAFFIC_LIGHT; // 현재 모드를 신호등 모드로 설정
    Serial.print("Mode: Traffic Light\n");
    analogWrite(redPin, map(potValue, 0, 1023, 0, 255)); // 가변저항 값에 따라 밝기 조절
    return true;
  }
  return false;
}

// 빨간불 Task가 비활성화될 때 실행되는 함수
void redTaskOnDisable() {
  Serial.println("Red Task Disabled");
  Serial.print("RL\n"); // 빨간불 꺼짐
  digitalWrite(redPin, LOW); // 빨간불 LED 끄기
  yellowTask.restartDelayed(); // 노란불 Task 시작
}

// 노란불 Task가 활성화될 때 실행되는 함수
bool yellowTaskOnEnable() {
  if (!button2Pressed && !button3Pressed) { // 버튼 2와 3이 눌리지 않은 경우에만 실행
    Serial.println("Yellow Task Enabled");
    Serial.print("Y\n"); // 노란불 켜짐
    analogWrite(yellowPin, map(potValue, 0, 1023, 0, 255)); // 가변저항 값에 따라 밝기 조절
    return true;
  }
  return false;
}

// 노란불 Task가 비활성화될 때 실행되는 함수
void yellowTaskOnDisable() {
  Serial.println("Yellow Task Disabled");
  Serial.print("YL\n"); // 노란불 꺼짐
  digitalWrite(yellowPin, LOW); // 노란불 LED 끄기
  greenTask.restartDelayed(); // 초록불 Task 시작
}

// 초록불 Task가 활성화될 때 실행되는 함수
bool greenTaskOnEnable() {
  if (!button2Pressed && !button3Pressed) { // 버튼 2와 3이 눌리지 않은 경우에만 실행
    Serial.println("Green Task Enabled");
    Serial.print("G\n"); // 초록불 켜짐
    analogWrite(greenPin, map(potValue, 0, 1023, 0, 255)); // 가변저항 값에 따라 밝기 조절
    return true;
  }
  return false;
}

// 초록불 Task가 비활성화될 때 실행되는 함수
void greenTaskOnDisable() {
  Serial.println("Green Task Disabled");
  Serial.print("GL\n"); // 초록불 꺼짐
  digitalWrite(greenPin, LOW); // 초록불 LED 끄기
  greenBlinkTask.restartDelayed(); // 초록불 깜빡임 Task 시작
}

// 초록불 깜빡임 Task가 활성화될 때 실행되는 함수
bool greenBlinkTaskOnEnable() {
  if (!button2Pressed && !button3Pressed) { // 버튼 2와 3이 눌리지 않은 경우에만 실행
    Serial.println("Green Blink Task Enabled");
    digitalWrite(greenPin, LOW); // 초록불 LED 초기화
    return true;
  }
  return false;
}

// 초록불 깜빡임 Task의 콜백 함수 (깜빡임 동작)
void greenBlinkTaskCallback() {
  static bool ledState = LOW; // LED 상태를 저장하는 변수
  ledState = !ledState; // LED 상태 토글
  analogWrite(greenPin, ledState ? map(potValue, 0, 1023, 0, 255) : 0); // 밝기 조절
  if (ledState) {
    Serial.println("Green Blink Task Callback");
    Serial.print("G\n"); // 초록불 켜짐
  } else {
    Serial.print("GL\n"); // 초록불 꺼짐
  }
}

// 초록불 깜빡임 Task가 비활성화될 때 실행되는 함수
void greenBlinkTaskOnDisable() {
  Serial.println("Green Blink Task Disabled");
  Serial.print("GL\n"); // 초록불 꺼짐
  digitalWrite(greenPin, LOW); // 초록불 LED 끄기
  yellowAfterGreenBlinkTask.restartDelayed(); // 초록불 깜빡임 후 노란불 Task 시작
}

// 초록불 깜빡임 후 노란불 Task가 활성화될 때 실행되는 함수
bool yellowAfterGreenBlinkTaskOnEnable() {
  if (!button2Pressed && !button3Pressed) { // 버튼 2와 3이 눌리지 않은 경우에만 실행
    Serial.println("Yellow After Green Blink Task Enabled");
    Serial.print("Y\n"); // 노란불 켜짐
    analogWrite(yellowPin, map(potValue, 0, 1023, 0, 255)); // 가변저항 값에 따라 밝기 조절
    return true;
  }
  return false;
}

// 초록불 깜빡임 후 노란불 Task가 비활성화될 때 실행되는 함수
void yellowAfterGreenBlinkTaskOnDisable() {
  Serial.println("Yellow After Green Blink Task Disabled");
  Serial.print("YL\n"); // 노란불 꺼짐
  digitalWrite(yellowPin, LOW); // 노란불 LED 끄기
  redTask.restartDelayed(); // 빨간불 Task 시작
}

// 빨간불 깜빡임 Task의 콜백 함수 (긴급 모드)
void redBlinkTaskCallback() {
  static bool ledState = LOW; // LED 상태를 저장하는 변수
  ledState = !ledState; // LED 상태 토글
  analogWrite(redPin, ledState ? map(potValue, 0, 1023, 0, 255) : 0); // 밝기 조절
  if (ledState) {
    Serial.println("Red Blink Task Callback");
    Serial.print("R\n"); // 빨간불 켜짐
  } else {
    Serial.print("RL\n"); // 빨간불 꺼짐
  }
}

// 모든 LED 깜빡임 Task의 콜백 함수
void allLedsBlinkTaskCallback() {
  static bool ledState = LOW; // LED 상태를 저장하는 변수
  ledState = !ledState; // LED 상태 토글
  int brightness = ledState ? map(potValue, 0, 1023, 0, 255) : 0; // 밝기 조절
  analogWrite(redPin, brightness);
  analogWrite(yellowPin, brightness);
  analogWrite(greenPin, brightness);
  if (ledState) {
    Serial.println("All LEDs Blink Task Callback");
    Serial.print("R\nY\nG\n"); // 모든 LED 켜짐
  } else {
    Serial.print("RL\nYL\nGL\n"); // 모든 LED 꺼짐
  }
}

// 가변저항 값을 읽고 출력하는 Task의 콜백 함수
void readPotentiometerTaskCallback() {
  int currentPotValue = analogRead(potPin); // 현재 가변저항 값 읽기

  // 값이 변경된 경우에만 출력
  if (currentPotValue != previousPotValue) {
    previousPotValue = currentPotValue; // 이전 값 업데이트
    Serial.print("potValue:" + String(currentPotValue) + "\n"); 
  }
}

// 모든 Task를 추가하는 함수
void SchAddTask() {
  runner.addTask(redTask);
  runner.addTask(yellowTask);
  runner.addTask(greenTask);
  runner.addTask(greenBlinkTask);
  runner.addTask(yellowAfterGreenBlinkTask);
  runner.addTask(redBlinkTask);
  runner.addTask(allLedsBlinkTask);


}

// 모든 Task를 삭제하는 함수
void SchAllDeleteTask() {
  runner.deleteTask(redTask);
  runner.deleteTask(yellowTask);
  runner.deleteTask(greenTask);
  runner.deleteTask(greenBlinkTask);
  runner.deleteTask(yellowAfterGreenBlinkTask);
  runner.deleteTask(redBlinkTask);
  runner.deleteTask(allLedsBlinkTask);

  AllLedsOff(); // 모든 LED 끄기
}

// 모든 LED를 끄는 함수
void AllLedsOff() {
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);

  Serial.print("RL\nYL\nGL\n"); // 모든 LED 꺼짐
}