# ESC_W3_traffic_light_gesture

# 프로젝트 시연 영상

아래 영상에서 프로젝트의 동작을 확인할 수 있습니다.

[![프로젝트 시연 영상](https://img.youtube.com/vi/VpENFWReUGU/0.jpg)](https://youtu.be/VpENFWReUGU)

[👉 영상 링크 바로가기](https://youtu.be/VpENFWReUGU)


# 🚦 1. ESC_W1_traffic_light_gesture 사용 방법

## 1️⃣ 프로젝트 개요
이 프로젝트는 **Arduino와 p5.js**를 이용하여 신호등을 제어하고,  
웹 UI에서 신호등 상태를 실시간으로 확인하고 손동작으로 조작할 수 있도록 설계되었습니다.  

---


## 2️⃣ 필요한 준비물
### 📌 하드웨어 목록
- **Arduino Uno**  
- **LED 3개** (빨강, 노랑, 초록)  
- **330Ω 저항 3개**  
- **푸시 버튼 3개**  
- **가변저항 1개**  
- **점퍼 와이어**  


---

## 3️⃣ 하드웨어 연결 방법
![image](https://github.com/user-attachments/assets/59849f39-9fe2-4b11-8119-a3a66598954c)
### 🚥 LED 연결
| LED 색상 | 아두이노 핀 | 연결 방법 |
|----------|----------|---------|
| 🔴 빨간 LED | 11번 핀 | 330Ω 저항 → LED(+) → GND |
| 🟡 노란 LED | 9번 핀  | 330Ω 저항 → LED(+) → GND |
| 🟢 초록 LED | 10번 핀 | 330Ω 저항 → LED(+) → GND |

### 🔘 버튼 연결 (내부 풀업 저항 사용)
| 버튼 기능 | 아두이노 핀 | 연결 방법 |
|----------|----------|---------|
| 긴급 모드 | 2번 핀 | 한쪽 다리를 GND, 대각선 다리를 2번 핀 |
| LED 깜빡임 | 3번 핀 | 한쪽 다리를 GND, 대각선 다리를 3번 핀 |
| 시스템 ON/OFF | 4번 핀 | 한쪽 다리를 GND, 대각선 다리를 4번 핀 |

### 🎛️ 가변저항 연결 (A0 핀 사용)
| 가변저항 단자 | 연결 핀 |
|-------------|--------|
| 중앙 단자 | A0 핀 |
| 한쪽 단자 | 5V |
| 반대쪽 단자 | GND |

**회로도 사진에서는 스위치가 가변저항을 대신합니다.**


---
# 2. 아두이노 코드 설명

## 🔹 개요
이 프로젝트는 **아두이노(Arduino)** 를 사용하여 LED 제어 및 버튼 입력을 처리하는 시스템입니다.  
버튼을 눌러 시스템을 ON/OFF 하거나, 특정 기능을 실행할 수 있습니다.  
또한, 가변저항을 이용해 특정 값을 조절할 수 있습니다.

---

## 🔹 주요 기능
### 1. LED 신호등
- **빨강 LED** : 시스템이 켜지면 빨간색 LED가 2초 점등됩니다.
- **노랑 LED** : 빨간색 LED,초록색 LED 점등 후에 노란색 LED가 0.5초 점등됩니다. 
- **초록 LED** : 노란색 LED점등 후에 초록색 LED가 2초 점등되고 1초안에 세번 깜빡입니다.
- 빨간색 -> 노란색 -> 초록색 ->초록색 3번 깜빡임 -> 노란색 순으로 무한 반복합니다. 

### 2. 버튼 입력 처리
- **버튼 2 (응급 모드 활성화)**
  - 버튼을 누르면 빨간 LED가 깜빡이며, 응급 모드가 활성화됩니다.
  - 비상상황임을 강조하기 위해 빨간LED가 켜져있는것이 아닌 깜빡이는 것으로 대체했습니다. 
- **버튼 3 (모든 LED 깜빡임)**
  - 버튼을 누르면 모든 LED가 깜빡입니다.
- **버튼 4 (시스템 ON/OFF)**
  - 버튼을 눌러 시스템을 켜거나 끌 수 있습니다.

### 3. 가변저항 활용
- 가변저항을 사용하여 LED 밝기를 조절할 수 있습니다. 
- 현재 코드에서는 가변저항 값을 읽어와 사용할 수 있도록 설정되어 있습니다.


---
# 🚦 3. p5.js 코드 설명

## 🔹 개요
이 프로젝트의 **p5.js 코드**는 **아두이노(Arduino)** 와 실시간으로 데이터를 주고받으며,  
웹 화면에서 **신호등 상태를 표시하고, 손모양으로 조작할 수 있는 기능**을 제공합니다.

---

## 🔹 주요 기능
### 1️⃣ **LED 상태(신호등)를 웹 화면에 표시**
- 아두이노에서 현재 켜진 **LED(빨강, 노랑, 초록)** 정보를 받아와 화면에 표시합니다.
- 신호등 모양의 UI를 구현하여, 현재 **어떤 LED가 켜져 있는지** 직관적으로 확인할 수 있습니다.

### 2️⃣ **현재 동작 중인 모드 표시**
- **손모양**에 따라 아두이노의 모드가 변경됩니다.
- 예를 들어:
  - **일반 모드** (기본 신호등 동작)
  - **긴급 모드** (빨간 LED 깜빡임)
  - **전체 LED Blinking 모드**
  - **시스템 ON/OFF 상태**
- 현재 어떤 모드인지 **텍스트로 웹에 표시**합니다.

### 3️⃣ **가변저항 값을 웹에서 원형 게이지로 표시**
- 아두이노에서 읽어온 **가변저항(A0 핀) 값**을 웹에서 **원형 게이지(Circular Gauge)** 로 시각화합니다.
- 가변저항의 값을 변환하여, 사용자가 **LED 밝기를 직관적으로 확인**할 수 있습니다.

### 4 기능 설명(추가된내용)
이 프로젝트는 손 모양을 이용하여 신호등 모드 변경과 LED 지속 시간을 조절할 수 있습니다.

### 🚦 모드 변경
| 모드명 | 손 모양 |
|--------|---------------------------|
| **신호등 모드** | 손가락 5개를 모두 편 상태 🖐 |
| **이멀전씨 모드** | 검지만 펴고 나머지는 접기 ☝ |
| **블링킹 모드** | 검지 + 중지만 펴고 나머지는 접기 ✌ |
| **시스템 오프** | 검지 + 중지 + 약지만 펴고 나머지는 접기 ✋ |

### 🎛 슬라이더 조절 (LED 지속 시간)
| 조작 | 왼손 | 오른손 |
|------|------|------|
| **빨간색 슬라이더 UP** | 주먹 ✊ | 떰즈업 👍 |
| **빨간색 슬라이더 DOWN** | 주먹 ✊ | 떰즈다운 👎 |
| **노란색 슬라이더 UP** | 샤카 🤙 (엄지+새끼) | 떰즈업 👍 |
| **노란색 슬라이더 DOWN** | 샤카 🤙 (엄지+새끼) | 떰즈다운 👎 |
| **초록색 슬라이더 UP** | 오케이(OK) 모양 👌 | 떰즈업 👍 |
| **초록색 슬라이더 DOWN** | 오케이(OK) 모양 👌 | 떰즈다운 👎 |

---

---

## 🔹 아두이노와의 실시간 데이터 송수신
이 모든 기능은 **아두이노와 웹(p5.js) 간의 실시간 데이터 통신**을 통해 이루어집니다.
- **아두이노 → p5.js** : LED 상태, 현재 모드, 가변저항 값 전송
- **p5.js → 아두이노** : LED 점등 시간 전송

이를 통해 웹 UI와 물리적인 신호등이 **실시간으로 연동**됩니다. 🚀

---

## 🔹 결론
이 p5.js 프로젝트는 **웹과 아두이노를 연결하여 신호등을 직관적으로 제어**할 수 있도록 합니다.  
- 🌟 신호등 상태 시각화  
- 🚥 모드 변경 확인  
- 🎛️ 가변저항 값 시각적 표시  
- 🕹️ 손모양으로 신호등 점등 시간 조절  

**이 모든 기능을 통해, 더 편리하고 직관적인 신호등 제어 시스템을 구축할 수 있습니다!**  










