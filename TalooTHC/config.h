
//#include <EEPROM.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Helvetica; font-size: 24px; display: inline-block; text-align: center;}
    body {max-width: 600px; margin:0px auto;}
    span {color: red;}
    div  {margin-top: 10px;}
    input[type=text] {height: 40px; font-size: 20px; width: 150px; direction: rtl; padding: 2px 5px;}
    input[type=button] {background-color: #4CAF50; color: white; border: none; cursor: pointer; font-size: 24px; padding: 10px 30px; }
    button {background: #e01a22; border: none; padding: 14px 20px; font-size: 24px; color: white; font-weight: 800; cursor: pointer; width: 200px;}
    button.ana {background: #4CAF50;}
    butto.capan {background: #e01a22;}
  </style>
</head>
<body>
  <div>
    <button type="button" class="ana" onclick="showTab(0)">Analog</button>
    <button type="button" class="capa" onclick="showTab(1)">Capacitance</button>
  </div>
  <div id="tab_ana">
    <h2>Analog to PWM</h2>
    <p>Frequency range <span id="s_min_freq">%FREQ_MIN_STYLE%</span> ~ <span id="s_max_freq">%FREQ_MAX_STYLE%</span></p>
    <div>
      <label>Min Frequency(Hz)</label>
      <input type="text" id="min_freq" value="%MINFREQVALUE%"/>
      <input type="button" value="Set" onclick="set_min()"/>
    </div>
    <div>
      <label>Max Frequency(Hz)</label>
      <input type="text" id="max_freq" value="%MAXFREQVALUE%"/>
      <input type="button" value="Set" onclick="set_max()"/>
    </div>
  </div>
  
  <div id="tab_capa">
    <h2>Capacitance to PWM</h2>
    <p>Min Value: <span id="s_min_capa">%CAPA_MIN_STYLE%</span><p>
    <p>Max Value: <span id="s_max_capa">%CAPA_MAX_STYLE%</span><p>

    <p>Current Value: <span id="s_capa">%CAPA_VALUE%</span><p>
    <div>
      <button type="button" onclick="set_capamin()">Set Min</button>
      <button type="button" onclick="set_capamax()">Set Max</button>
    </div>
  </div>
<script>

var mode = 0;   // 0 - analog     1 - capacitance

function showTab(tab){
  if ( tab == 0 ) {
    document.getElementById("tab_ana").style.display = "";
    document.getElementById("tab_capa").style.display = "none";
  } else {
    document.getElementById("tab_ana").style.display = "none";
    document.getElementById("tab_capa").style.display = "";
  }
  set_mode(tab);
}

function set_min(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if ( this.responseText == "FAILED" ) {
        alert("Failed! Min Frequency is invalid!");
      } else {
        document.getElementById("s_min_freq").innerHTML = this.responseText;
      }
    }
  };
  var value = document.getElementById("min_freq").value;
  xhr.open("GET", "/setmin?val=" + value, true);
  xhr.send();
  
}

function set_max(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if ( this.responseText == "FAILED" ) {
        alert("Failed! Max Frequency is invalid!");
      } else {
        document.getElementById("s_max_freq").innerHTML = this.responseText;
      }
    }
  };
  var value = document.getElementById("max_freq").value;
  xhr.open("GET", "/setmax?val=" + value, true);
  xhr.send();
}

function set_capamax(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if ( this.responseText == "FAILED" ) {
        alert("Failed! Max Capacitance is invalid!");
      } else {
        document.getElementById("s_max_capa").innerHTML = this.responseText;
      }
    }
  };
  xhr.open("GET", "/setcapamax", true);
  xhr.send();
}

function set_capamin(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if ( this.responseText == "FAILED" ) {
        alert("Failed! Min Capacitance is invalid!");
      } else {
        document.getElementById("s_min_capa").innerHTML = this.responseText;
      }
    }
  };
  xhr.open("GET", "/setcapamin", true);
  xhr.send();
}

function set_mode(n) {
  cur_mode = n;

  var xhr = new XMLHttpRequest();  
  xhr.open("GET", "/setmode?val=" + cur_mode, true);
  xhr.send();
  
}

function get_capa() {
  if ( cur_mode == 0 ) {
    return;
  }

  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    console.log(this.readyState);
    console.log(this.status);
    if (this.readyState == 4 && this.status == 200) {
      if ( this.responseText == "FAILED" ) {
        document.getElementById("s_capa").innerHTML = "0";
      } else {
        document.getElementById("s_capa").innerHTML = this.responseText;
      }
    } else if (this.readyState == 4 && this.status == 500) {
      document.getElementById("s_capa").innerHTML = "invalid!";
    }
  };
  
  xhr.open("GET", "/getcapa", true);
  xhr.send();
}

showTab(0);
setInterval(get_capa, 1000);

</script>
</body>
</html>
)rawliteral";



void loadConfig() {
  
}

void saveConfig() {
  
}
