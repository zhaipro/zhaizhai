// http://www.taichi-maker.com/homepage/iot-development/iot-dev-reference/esp8266-c-plus-plus-reference/esp8266webserver/
// https://www.runoob.com/try/try.php?filename=tryhtml_basic_link
// https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-arduino-ide
// https://randomnerdtutorials.com/esp32-servo-motor-web-server-arduino-ide/
#include <WiFi.h>
#include <WebServer.h>
#include <Servo.h>  //调用舵机库

#ifndef LED_BUILTIN
// ?@_@
#define LED_BUILTIN 13
#endif

//设置您的WiFi接入信息
const char* ssid = "zhaipro";
const char* password = "bow-wow";

WebServer server(80);

void homepage() {
  const char *html = "<meta name='viewport' content='width=device-width'>      \
<script>        \
    function do_fetch(pathname, params=null, kws=null) {        \
        if (window.location.origin.startsWith('file')) {        \
          return;       \
        }       \
        var url = new URL(pathname, window.location.origin);    \
        if (params == null) {   \
            params = [];        \
        }       \
        for (let i in params) { \
            var k = params[i];  \
            var v = document.getElementById(k).value;   \
            url.searchParams.append(k, v);      \
        }       \
        if (kws == null) {      \
            kws = {}    \
        }       \
        for (let k in kws) {    \
            url.searchParams.append(k, kws[k]); \
        }       \
        fetch(url);     \
    }   \
        \
    function do_init() {        \
        var pathname = '/init'; \
        var params = ['rf', 'rb', 'lb', 'lf', 'interval', 'dynamics', 'ad_dynamics'];   \
        do_fetch(pathname, params);     \
    }   \
        \
    function cmd(status, ad=null) {     \
        var pathname = '/cmd';  \
        var kws = {'status': status};   \
        if (ad) {       \
          kws['ad'] = ad;       \
        }       \
        do_fetch(pathname, null, kws);  \
    }   \
</script>       \
        \
interval: <input type='number' id='interval' value=5 max=1000> <br>     \
dynamics: <input type='number' id='dynamics' value=30 max=90> <br>      \
rf: <input type='number' id='rf' value=-13> <br>        \
rb: <input type='number' id='rb' value=-13> <br>        \
lb: <input type='number' id='lb' value=-13> <br>        \
lf: <input type='number' id='lf' value=-3> <br> \
ad_dynamics: <input type='number' id='ad_dynamics' value=5> <br>        \
<button onmouseup=do_init()>init</button> <br>  \
<br>    \
        \
<button onmouseup=cmd(0)>stand</button> <br>    \
<button onmouseup=cmd(32)>down</button> <br>    \
<br>    \
        \
<button onmouseup=cmd(64)>front</button> <br>   \
<button onmouseup=cmd(128)>left</button>        \
<button onmouseup=cmd(160)>right</button> <br>  \
<button onmouseup=cmd(96)>back</button> <br>    \
        \
<!-- https://github.com/yoannmoinet/nipplejs -->        \
<div id='zone_joystick'>        \
<script src='https://cdnjs.cloudflare.com/ajax/libs/nipplejs/0.7.3/nipplejs.min.js'></script>   \
<script>        \
var options = { \
  zone: document.getElementById('zone_joystick'),       \
  color: 'blue',        \
  mode: 'static',       \
  size: 300,    \
  position: {   \
    left: '50%',        \
    top: '70%'  \
  },    \
};      \
var g_ad = 0;   \
manager = nipplejs.create(options);     \
manager.on('end', function (evt, data) {        \
    g_ad = 0;   \
    cmd(0);     \
  });   \
manager.on('dir', function (evt, data) {        \
    console.debug(JSON.stringify(data));        \
    if (data.direction.angle == 'up') { \
      cmd(2);   \
    } else {    \
      g_ad = 0; \
    }   \
    if (data.direction.angle == 'down') {       \
      cmd(4);   \
    } else if (data.direction.angle == 'left') {        \
      cmd(8);   \
    } else if (data.direction.angle == 'right') {       \
      cmd(16);  \
    }   \
  });   \
manager.on('move', function (evt, data) {       \
    if (data.direction.angle != 'up') { \
      return;   \
    }   \
    var degree = 180 - data.angle.degree;       \
    var ad = 0; \
    if (degree < 90 - 15) {     \
      ad = -1;  \
    } else if (degree > 90 + 15) {      \
      ad = 1;   \
    }   \
    if (ad != g_ad) {   \
      g_ad = ad;        \
      cmd(2, ad);       \
    }   \
  });   \
</script>";
  server.send(200, "text/html", html);
}

class Servos {
  // 舵机对象，顺时针从右前方开始
  Servo m_servo_rf;   // 右前方
  Servo m_servo_rb;   // 右后方
  Servo m_servo_lb;   // 左后方
  Servo m_servo_lf;   // 左前方
public:
  // 偏移值
  int m_rf = 0;
  int m_rb = 0;
  int m_lb = 0;
  int m_lf = 0;
  // 上一次的姿态，逻辑值
  int m_rf_last = 90;
  int m_rb_last = 90;
  int m_lb_last = 90;
  int m_lf_last = 90;

  void attach(int rf, int rb, int lb, int lf) {
    m_servo_rf.attach(rf);  // 设置舵机控制针脚接引脚
    m_servo_rb.attach(rb);
    m_servo_lb.attach(lb);
    m_servo_lf.attach(lf);
  }

  void _move(int rf, int rb, int lb, int lf, int velocity) {
    int angle = 0;  // 最大旋转角度
    angle = max(abs(rf - m_rf_last), angle);
    angle = max(abs(rb - m_rb_last), angle);
    angle = max(abs(lb - m_lb_last), angle);
    angle = max(abs(lf - m_lf_last), angle);
    for (int i = 1; i <= angle; i ++) {
      m_servo_rf.write(m_rf + m_rf_last + (rf - m_rf_last) * i / angle);
      m_servo_rb.write(m_rb + m_rb_last + (rb - m_rb_last) * i / angle);
      m_servo_lb.write(m_lb + m_lb_last + (lb - m_lb_last) * i / angle);
      m_servo_lf.write(m_lf + m_lf_last + (lf - m_lf_last) * i / angle);
      delay(velocity);
    }
    m_rf_last = rf;
    m_rb_last = rb;
    m_lb_last = lb;
    m_lf_last = lf;
  }

  void move(int rf, int rb, int lb, int lf, int velocity) {
    _move(90 + rf, 90 + rb, 90 + lb, 90 + lf, velocity);
  }
};

int g_interval = 5;
int g_dynamics = 30;
int g_ad_dynamics = 5;
Servos servos;

int get_arg(const char *name, int default_value=0) {
  if (server.hasArg(name)) {
    return server.arg(name).toInt();
  }
  return default_value;
}

void do_init() {
  servos.m_rf = get_arg("rf");
  servos.m_rb = get_arg("rb");
  servos.m_lb = get_arg("lb");
  servos.m_lf = get_arg("lf");
  g_interval = get_arg("interval", 5);
  g_dynamics = get_arg("dynamics", 30);
  g_ad_dynamics = get_arg("ad_dynamics", 5);
  servos.move(0, 0, 0, 0, 0);

  server.send(200, "text/plain", "");
}

void move(int w, int s, int a, int d) {
  // 前后左右移动的一般规律：
  // 1. 准备
  servos.move((w - a) * g_dynamics, (s + d) * g_dynamics, -(w + a) * g_dynamics, -(s - d) * g_dynamics, g_interval);
  // 2. 下发力
  servos.move(((w - a) - (s + d)) * g_dynamics, (-(w - a) + (s + d)) * g_dynamics, (-(w + a) + (s - d)) * g_dynamics, ((w + a) - (s - d)) * g_dynamics, g_interval);
  // 3. 上发力
  servos.move(-(s + d) * g_dynamics, -(w - a) * g_dynamics, (s - d) * g_dynamics, (w + a) * g_dynamics, g_interval);
  // 4. 立正
  servos.move(0, 0, 0, 0, g_interval);
}

void move1(int w, int s, int ad=0) {
  int l_dynamics = g_dynamics;
  int r_dynamics = g_dynamics;
  if (ad < 0) {
    l_dynamics = g_dynamics + ad * g_ad_dynamics;
  } else if (ad > 0) {
    r_dynamics = g_dynamics - ad * g_ad_dynamics;
  }
  // 1
  servos.move(w * r_dynamics, s * r_dynamics, -w * l_dynamics, -s * l_dynamics, g_interval);
  // 2
  servos.move((w - s) * r_dynamics, (s - w) * r_dynamics, (s - w) * l_dynamics, (w - s) * l_dynamics, g_interval);
  // 2.5
  servos.move(-s * r_dynamics, -w * r_dynamics, s * l_dynamics, w * l_dynamics, g_interval);
  // 3
  servos.move(0, 0, 0, 0, g_interval);
}

void move2(int w, int s, int ad=0) {
  move1(s, w, ad);
}

void rotation1(int a, int d) {
  // 1
  servos.move(-a * g_dynamics, d * g_dynamics, -a * g_dynamics, d * g_dynamics, g_interval);
  // 2
  servos.move(-(a + d) * g_dynamics, (a + d) * g_dynamics, -(a + d) * g_dynamics, (a + d) * g_dynamics, g_interval);
  // 3
  servos.move(-d * g_dynamics, a * g_dynamics, -d * g_dynamics, a * g_dynamics, g_interval);
  // 4
  servos.move(0, 0, 0, 0, g_interval);
}

void rotation2(int a, int d) {
  rotation1(-d, -a);
}

void stand() {
  servos._move(servos.m_rf_last, 90, 90, servos.m_lf_last, g_interval);
  servos._move(90, 90, 90, 90, g_interval);
}

void down() {
  servos._move(180, servos.m_rb_last, servos.m_lb_last, 0, g_interval);
  servos._move(180, 0, 180, 0, g_interval);
}

int g_status = 0;
/*
站立
趴到
前进1、2
后退1、2
伸展向前
伸展先后
*/

int g_ad = 0;

void cmd() {
  int status = get_arg("status", 0);
  g_ad = get_arg("ad", 0);
  g_status = status;
  if (status == B100000) {
    down();
  } else if (status == 0) {
    stand();
  } else if (status == B1000000) {
    servos._move(90, 45, 135, 90, g_interval);
  } else if (status == B1100000) {
    servos._move(135, 90, 90, 45, g_interval);
  } else if (status == B10000000) {
    servos._move(90, 90, 135, 45, g_interval);
  } else if (status == B10100000) {
    servos._move(135, 45, 90, 90, g_interval);
  }
  server.send(200, "text/plain", "");
}

void _loop() {
  if (g_status == B010) {          // 向前
    move1(1, 0, g_ad);
    g_status = B011;
  } else if (g_status == B011) {
    move2(1, 0, g_ad);
    g_status = B010;
  } else if (g_status == B100) {   // 向后
    move1(-1, 0, g_ad);
    g_status = B101;
  }else if (g_status == B101) {
    move2(-1, 0, g_ad);
    g_status = B100;
  } else if (g_status == B01000) {   // 向左
    rotation1(0, 1);
    g_status = B01001;
  }  else if (g_status == B01001) {
    rotation2(0, 1);
    g_status = B01000;
  } else if (g_status == B10000) {   // 向右
    rotation1(1, 0);
    g_status = B10001;
  } else if (g_status == B10001) { 
    rotation2(1, 0);
    g_status = B10000;
  } 
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  for (int i = 0; i < 3 && WiFi.status() != WL_CONNECTED; i++)
  {
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off by making the voltage LOW
    delay(1000);                      // wait for a second
    Serial.print(".");
  }
  // 我的手机热点
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin("zhaipro", "bow-wow");
  }
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off by making the voltage LOW
    delay(1000);                      // wait for a second
    Serial.print(".");
  }
  Serial.println(" connected");

  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());

  // 初始化WebServer
  server.on("/", homepage);
  server.on("/init", do_init);
  server.on("/cmd", cmd);
  server.begin();
  Serial.println("HTTP server started");

  servos.attach(2, 13, 32, 23);
}

void loop(void) {
  // 监听客户请求并处理
  server.handleClient();
  _loop();
}
