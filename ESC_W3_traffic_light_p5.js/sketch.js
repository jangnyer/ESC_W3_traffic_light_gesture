let port;
let connectBtn;
// LED 길이 변수 (단위: ms)
let redDuration = 2000; // 초기값 0.5초
let yellowDuration = 500; // 초기값 0.5초
let greenDuration = 2000; // 초기값 0.5초
let redState = false;
let yellowState = false;
let greenState = false;
let currentMode = "Mode: Traffic Light"; // Initial mode
let potValue = 0; // 가변 저항 값 저장 변수
let canvasHeight = 700; // 캔버스 높이
let handPose;
let video;
let hands = [];

function preload() {
  handPose = ml5.handPose();
}

function setup() {
  createCanvas(1200,1000); // 캔버스를 더 길게 설정

  port = createSerial(); // web serial controll object

  // 이전 연결 정보를 통해 자동으로 연결
  let usedPorts = usedSerialPorts();
  if (usedPorts.length > 0) {
    port.open(usedPorts[0], 9600); // 과거에 사용했던 포트에 9600 baud rate로 연결
  }

  // web serial connect button setting
  connectBtn = createButton("Connect to Arduino");
  connectBtn.position(50, 20);
  connectBtn.mousePressed(connectBtnClick);

  //웹캠
  video = createCapture(VIDEO, { flipped: true });
  video.size(640, 480);
  video.hide();
  handPose.detectStart(video, gotHands);

  // 🔥 1초마다 손 모양 감지
  setInterval(detectGestures, 1000);
}

function draw() {
  // 모드 텍스트 표시
  background(255); // 배경 초기화
  fill(0);
  textSize(24); // 큰 글씨
  textAlign(LEFT, CENTER);
  text(`${currentMode}`, 50, 70); // 연결 버튼 바로 아래에 표시


  let n = port.available(); // 수신된 데이터가 있으면
  if (n > 0) {
    let str = port.readUntil("\n"); // 개행문자까지 읽고
    let timestamp =
      nf(hour(), 2) + ":" + nf(minute(), 2) + ":" + nf(second(), 2); // 디버깅용 정보
    print(timestamp + "  " + nf(n, 2) + ":" + trim(str)); // 해당 정보를 콘솔에 보여줌 (디버깅용)

    // 가변 저항 값 추출
    if (str.startsWith("potValue:")) {
      let valueStr = str.split(":")[1]; // "potValue:900"에서 "900" 추출
      potValue = int(valueStr.trim()); // 문자열을 정수로 변환
    }

    // 시스템 OFF 처리
    if (str === "Mode: System Off\n") {
      currentMode = str.trim();

      redState = false;
      yellowState = false;
      greenState = false;


      // 화면 즉시 업데이트
      fill("gray");
      circle(100, 180, 40); // 빨간불
      circle(100, 240, 40); // 노란불
      circle(100, 300, 40); // 초록불

      return; // 다른 작업이 실행되지 않도록 종료
    }

    // 빨간불
    if (str === "R\n") redState = true;
    else if (str === "RL\n" || str === "Mode: System Off\n") redState = false;

    // 노란불
    if (str === "Y\n" ) yellowState = true;
    else if (str === "YL\n" || str === "Mode: System Off\n") yellowState = false;

    // 초록불
    if (str === "G\n" ) greenState = true;
    else if (str === "GL\n" || str === "Mode: System Off\n") greenState = false;

    // Update and display mode if str contains "Mode"
    if (str.startsWith("Mode")) {
      currentMode = str.trim(); // Update the current mode
    }
  }

  // 원형 게이지 표시
  fill(255); // 배경색으로 채움 (흰색)
  noStroke();
  ellipse(300, 200, 120, 120); // 원형 게이지 영역을 덮음

  // 게이지 값 계산
  let angle = map(potValue, 0, 1023, 0, TWO_PI); // 값을 각도로 변환

  // 원형 게이지 그리기
  noFill();
  stroke(200);
  strokeWeight(10);
  ellipse(300, 200, 100, 100); // 게이지 배경

  // 게이지 값 표시
  stroke(0, 255, 0);
  arc(300, 200, 100, 100, -HALF_PI, -HALF_PI + angle); // 게이지 값

  // 가변 저항 값 텍스트로 표시
  noStroke();
  fill(0);
  textSize(16);
  textAlign(CENTER, CENTER);
  text(potValue, 300, 200); // 중앙에 값 표시

  // changes button label based on connection status
  if (!port.opened()) {
    connectBtn.html("Connect to Arduino");
  } else {
    connectBtn.html("Disconnect");
  }

  // 신호등 상태 업데이트
    updateTrafficLight();

   //웹캠, 손 모양 표시
   image(video, 400, 0, 640, 480);
   for (let i = 0; i < hands.length; i++) {
     let hand = hands[i];
     for (let j = 0; j < hand.keypoints.length; j++) {
       let keypoint = hand.keypoints[j];
       fill(255, 0, 0);
       noStroke();
       circle(1040 - keypoint.x, keypoint.y, 10);
     }
   }
}


// ✅ 손 데이터 업데이트
function gotHands(results) {
  hands = results;
}

/********추가 작성 코드******************** */

// ✅ 특정 손가락이 펴져 있는지 확인
function isFingerExtended(hand, fingerTip, lowerJoint) {
  return hand.keypoints[fingerTip].y < hand.keypoints[lowerJoint].y - 5; // 5픽셀 여유
}

// ✅ 특정 손가락이 접혀 있는지 확인
function isFingerBent(hand, fingerTip, lowerJoint) {
  return hand.keypoints[fingerTip].y > hand.keypoints[lowerJoint].y + 5; // 5픽셀 여유
}

// ✅ 손가락 개수 세기 (몇 개가 펴져 있는지 확인)
function countExtendedFingers(hand) {
  if (!hand) return 0;
  let extendedCount = 0;

  [4, 8, 12, 16, 20].forEach((tip, i) => {
    if (isFingerExtended(hand, tip, tip - 2)) extendedCount++;
  });

  return extendedCount;
}

// ✅ 주먹 감지 (모든 손가락이 접혀 있는지 확인)
function isFist(hand) {
  if (!hand) return false;
  
  return isThumbBent(hand,"left") &&       // 엄지가 접혀 있어야 함
         isFingerBent(hand, 8, 6) &&  // 검지 접힘
         isFingerBent(hand, 12, 10) && // 중지 접힘
         isFingerBent(hand, 16, 14) && // 약지 접힘
         isFingerBent(hand, 20, 18);   // 새끼손가락 접힘
}


// ✅ 오케이(OK) 모양 감지
function isOKGesture(hand) {
  if (!hand) return false;
  let landmarks = hand.keypoints;

  let thumbTip = landmarks[4];
  let indexTip = landmarks[8];
  let thumbIndexClose = dist(thumbTip.x, thumbTip.y, indexTip.x, indexTip.y) < 30;

  return thumbIndexClose &&
         isFingerExtended(hand, 12, 10) && 
         isFingerExtended(hand, 16, 14) && 
         isFingerExtended(hand, 20, 18);
}

// ✅ 모드 감지 함수
function detectMode(hand) {
  if (!hand) return null;

  if (countExtendedFingers(hand) === 5) {
    sendValueToArduino("Mode: Traffic Light");
    currentMode="Mode: Traffic Light";
    return "🚦 모드 변경: 신호등 모드";
  }
  if (isFirstGesture(hand)) {
    sendValueToArduino("Mode: Emergency Mode");
    currentMode="Mode: Emergency Mode";
    return "🔵 모드 변경: 이멀전씨 모드";
  }
  if (isBlinkingMode(hand)) {
    sendValueToArduino("Mode: Blinking Mode");
    currentMode="Mode: Blinking Mode";
    return "🟡 모드 변경: 블링킹 모드";
  }
  if (isSystemOffMode(hand)) {
    currentMode="Mode: System Off";
    sendValueToArduino("Mode: System Off");
    return "🔴 모드 변경: 시스템 오프";
  }

  return null;
}

// ✅ 모드 감지 기능 통합
function detectGestures() {
  if (hands.length === 0) {
    console.log("⛔ 손이 감지되지 않음");
    return;
  }

  let leftHand = hands.length > 0 ? hands[0] : null;
  let rightHand = hands.length > 1 ? hands[1] : null;

  // ✅ 두 손이 감지되면 모드 변경 제외, 슬라이더 조절만 수행
  if (leftHand && rightHand) {
    let sliderAction = detectSliderControl(leftHand, rightHand);
    if (sliderAction) console.log(sliderAction);
    return;
  }

  // ✅ 한 손만 감지된 경우, 모드 변경 수행
  let mode = detectMode(leftHand) || detectMode(rightHand);
  if (mode) console.log(mode);
}

// ✅ 슬라이더 조작 감지 함수
function detectSliderControl(leftHand, rightHand) {
  debugHandState(leftHand, "왼손손 slider");
  if (!leftHand || !rightHand) return null;

  // 빨간불 슬라이더 조절
  if (isFist(leftHand) && isThumbsUp(rightHand)) {
    redDuration += 500; // 빨간불 길이 0.5초 증가
    sendValueToArduino(`red:${redDuration}`); // 아두이노로 값 전송
    return "🔺 빨간색 슬라이더 UP";
  }
  if (isFist(leftHand) && isThumbsDown(rightHand)) {
    redDuration = max(500, redDuration - 500); // 빨간불 길이 0.5초 감소 (최소값 500ms)
    sendValueToArduino(`red:${redDuration}`); // 아두이노로 값 전송
    return "🔻 빨간색 슬라이더 DOWN";
  }

  // 노란불 슬라이더 조절
  if (isShaka(leftHand) && isThumbsUp(rightHand)) {
    yellowDuration += 500; // 노란불 길이 0.5초 증가
    sendValueToArduino(`yellow:${yellowDuration}`); // 아두이노로 값 전송
    return "🔺 노란색 슬라이더 UP";
  }
  if (isShaka(leftHand) && isThumbsDown(rightHand)) {
    yellowDuration = max(500, yellowDuration - 500); // 노란불 길이 0.5초 감소 (최소값 500ms)
    sendValueToArduino(`yellow:${yellowDuration}`); // 아두이노로 값 전송
    return "🔻 노란색 슬라이더 DOWN";
  }

  // 초록불 슬라이더 조절
  if (isOKGesture(leftHand) && isThumbsUp(rightHand)) {
    greenDuration += 500; // 초록불 길이 0.5초 증가
    sendValueToArduino(`green:${greenDuration}`); // 아두이노로 값 전송
    return "🔺 초록색 슬라이더 UP";
  }
  if (isOKGesture(leftHand) && isThumbsDown(rightHand)) {
    greenDuration = max(500, greenDuration - 500); // 초록불 길이 0.5초 감소 (최소값 500ms)
    sendValueToArduino(`green:${greenDuration}`); // 아두이노로 값 전송
    return "🔻 초록색 슬라이더 DOWN";
  }

  return null;
}

// ✅ 기타 감지 함수들


// ✅ 이멀전씨 모드 감지 (검지만 펴고 나머지는 접힘)
function isFirstGesture(hand) {
  if (!hand) return false;

  return isFingerExtended(hand, 8, 6) &&  // 검지 펴짐
         isThumbBent(hand,"right") &&             // 엄지 접힘 (손바닥 안쪽으로)
         isFingerBent(hand, 12, 10) &&    // 중지 접힘
         isFingerBent(hand, 16, 14) &&    // 약지 접힘
         isFingerBent(hand, 20, 18);      // 새끼손가락 접힘

}

// ✅ 블링킹 모드 감지 (검지 + 중지만 펴고 나머지는 접힘)
function isBlinkingMode(hand) {
  if (!hand) return false;

  return isFingerExtended(hand, 8, 6) &&  // 검지 펴짐
         isFingerExtended(hand, 12, 10) && // 중지 펴짐
         isThumbBent(hand,"right") &&             // 엄지 접힘
         isFingerBent(hand, 16, 14) &&    // 약지 접힘
         isFingerBent(hand, 20, 18);      // 새끼손가락 접힘
}

// ✅ 시스템 오프 모드 감지 (검지 + 중지 + 약지만 펴고 나머지는 접힘)
function isSystemOffMode(hand) {
  if (!hand) return false;

  return isFingerExtended(hand, 8, 6) &&  // 검지 펴짐
         isFingerExtended(hand, 12, 10) && // 중지 펴짐
         isFingerExtended(hand, 16, 14) && // 약지 펴짐
         isThumbBent(hand,"right") &&             // 엄지 접힘
         isFingerBent(hand, 20, 18);      // 새끼손가락 접힘
}

function isShaka(hand) {
  if (!hand) return false;
  return countExtendedFingers(hand) === 2 &&
         !isThumbBent(hand, "left") &&
         isFingerExtended(hand, 20, 18);
}

function isThumbsUp(hand) {
  if (!hand) return false;
  return isFingerExtended(hand, 4, 3);
}

function isThumbsDown(hand) {
  if (!hand) return false;
  return isFingerBent(hand, 4, 3);
}
// ✅ 엄지가 접혀 있는지 확인하는 함수 (다른 손가락과 구분됨)
function isThumbBent(hand,leftright) {
  if (!hand) return false;
  let thumbTip = hand.keypoints[4];  // 엄지 끝부분
  let thumbBase = hand.keypoints[2]; // 엄지 관절
  if(leftright=="left"){
    return thumbTip.x > thumbBase.x 
  }
  else if(leftright=="right"){
  return thumbTip.x < thumbBase.x 
  }
  else{
    return true;
  }
}

// ✅ 디버깅용 로그 추가 (손가락 상태 확인)
function debugHandState(hand, modeName) {
  if (!hand) return;
  let landmarks = hand.keypoints;
  console.log(`🖐 ${modeName} 디버깅:`);
  console.log(`👍 엄지 (${isThumbBent(hand,"left") ? "접힘" : "펴짐"})`);
  console.log(`☝️ 검지 (${isFingerExtended(hand, 8, 6) ? "펴짐" : "접힘"})`);
  console.log(`✌️ 중지 (${isFingerExtended(hand, 12, 10) ? "펴짐" : "접힘"})`);
  console.log(`🤟 약지 (${isFingerExtended(hand, 16, 14) ? "펴짐" : "접힘"})`);
  console.log(`🖖 새끼손가락 (${isFingerExtended(hand, 20, 18) ? "펴짐" : "접힘"})`);
  console.log(`-----------------------------------`);
}



//**************이하 1주차 신호등 과제 코드와 동일*************************** */
function updateTrafficLight() {
  // 빨간불
  fill(redState ? "red" : "gray");
  circle(100, 180, 40);

  // 노란불
  fill(yellowState ? "yellow" : "gray");
  circle(100, 240, 40);

  // 초록불
  fill(greenState ? "green" : "gray");
  circle(100, 300, 40);

  // 🔥 현재 LED 길이 값 표시
  // 배경 칠하기
  fill(255); // 흰색 배경
  noStroke();
  rect(50, 350, 200, 100); // 신호등 아래 영역

  // 텍스트 표시
  fill(0); // 검은색 글씨
  textSize(16);
  textAlign(LEFT, CENTER);
  text(`Red Duration: ${redDuration / 1000}s`, 60, 370); // 빨간불 길이
  text(`Yellow Duration: ${yellowDuration / 1000}s`, 60, 400); // 노란불 길이
  text(`Green Duration: ${greenDuration / 1000}s`, 60, 430); // 초록불 길이
}

function connectBtnClick() {
  if (!port.opened()) {
    port.open(9600); // 9600 baudRate
  } else {
    port.close();
  }
}


function sendValueToArduino(value) {
  let data = `${value}\n`; // "2000\n" 형식으로 데이터 전송
  port.write(data);
}

