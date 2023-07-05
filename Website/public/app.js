function getTrilateration(position1, position2, position3) {
  var xa = position1.x;
  var ya = position1.y;
  var xb = position2.x;
  var yb = position2.y;
  var xc = position3.x;
  var yc = position3.y;
  var ra = position1.distance;
  var rb = position2.distance;
  var rc = position3.distance;

  var S = (Math.pow(xc, 2.) - Math.pow(xb, 2.) + Math.pow(yc, 2.) - Math.pow(yb, 2.) + Math.pow(rb, 2.) - Math.pow(rc, 2.)) / 2.0;
  var T = (Math.pow(xa, 2.) - Math.pow(xb, 2.) + Math.pow(ya, 2.) - Math.pow(yb, 2.) + Math.pow(rb, 2.) - Math.pow(ra, 2.)) / 2.0;
  var y = ((T * (xb - xc)) - (S * (xb - xa))) / (((ya - yb) * (xb - xc)) - ((yc - yb) * (xb - xa)));
  var x = ((y * (ya - yb)) - T) / (xb - xa);

  return {
      x: x,
      y: y
  };
}

function removeItemOnce(arr, value) {
  var index = arr.indexOf(value);
    if (index > -1) {
      arr.splice(index,1);
    }
    return arr;
  };

let tracker_1 = document.querySelector('.tracker');
let tracker_2 = document.querySelector('.tracker-2');

// Database Paths
var dataFloatPath = 'device1/activity';
var datax1 = 'device1/x_coordinate';
var datay1 = 'device1/y_coordinate';
var datax2 = 'device2/x_coordinate';
var datay2 = 'device2/y_coordinate';
var batt = 'device1/battery'

// Get a database reference 
const databaseFloat = database.ref(dataFloatPath);
const databasex1 = database.ref(datax1);
const databasey1 = database.ref(datay1);
const databasex2 = database.ref(datax2);
const databasey2 = database.ref(datay2);
const databasebat = database.ref(batt);

// Variables to save database current values
var floatReading;
var x1Reading;
var y1Reading;
var x2Reading;
var y2Reading;
var batteryReading;

//Various Var
var delay = 1000;
var i;
var j;

// var act_1 = document.querySelector(".logo-act-1");
// var act_2 = document.querySelector(".logo-act-2");
// var act_3 = document.querySelector(".logo-act-3");
const map = document.querySelector(".map");


// Attach an asynchronous callback to read the data
databaseFloat.on('value', (snapshot) => {
  floatReading = snapshot.val();
  console.log(floatReading);
  document.getElementById("reading-activity").innerHTML = floatReading;
  document.getElementById("activity").value = floatReading;
  if (floatReading == "Jalan"){
    //act_1.style.border = "6px solid #00b2ca";
    //act_2.style.border = "6px solid #353535";
    //act_3.style.border = "6px solid #353535";
    document.getElementById("imgid").src = "./assets/img/orang.png";
  }
  else if (floatReading == "Jatuh"){
    // act_1.style.border = "6px solid #353535";
    // act_2.style.border = "6px solid #00b2ca";
    // act_3.style.border = "6px solid #353535";
    document.getElementById("imgid").src = "./assets/img/falling.png";
  }
  else if (floatReading == "Tidur"){
    // act_1.style.border = "6px solid #353535";
    // act_2.style.border = "6px solid #353535";
    // act_3.style.border = "6px solid #00b2ca";
    document.getElementById("imgid").src = "./assets/img/sleeping.png";
  };
}, 
(errorObject) => {
  console.log('The read failed: ' + errorObject.name);
});

databasebat.on('value', (snapshot) => {
  batteryReading = snapshot.val();
  console.log(batteryReading);
  document.getElementById("reading-battery").innerHTML = batteryReading;
  if (batteryReading < 100 && batteryReading > 75){
    document.getElementById("persen").src = "./assets/img/battery4.png";
  }
  else if (batteryReading < 75 && batteryReading > 50){
    document.getElementById("persen").src = "./assets/img/battery3.png";
  }
  else if (batteryReading < 50 && batteryReading > 25){
    document.getElementById("persen").src = "./assets/img/battery2.png";
  }
  else if (batteryReading < 25 && batteryReading > 10){
    document.getElementById("persen").src = "./assets/img/battery1.png";
  }
  else if (batteryReading < 10 && batteryReading > 0){
    document.getElementById("persen").src = "./assets/img/battery.png";
  };
}, 
(errorObject) => {
  console.log('The read failed: ' + errorObject.name);
});

databasex1.on('value', (snapshot) => {
  x1Reading = snapshot.val();
  databasey1.on('value',(snapshot) =>{
    y1Reading=snapshot.val();
    var x_11 = Math.floor(Math.abs(x1Reading / 7 * 100))+"%";
    var y_11 = Math.floor(Math.abs(y1Reading /8* 100))+"%";
    var x_1 = Math.floor(Math.abs(x1Reading / 7 * 100));
    var y_1 = Math.floor(Math.abs(y1Reading /8* 100));
    document.getElementById("lat").value = x1Reading;
    document.getElementById("lang").value = y1Reading;
    console.log(x_1);
    console.log(y_1);
    if (x_1>57 && x_1 < 100 && y_1 < 50) {
      document.getElementById("lokasi").value = "Booth 1";
      document.getElementById("reading-location").innerHTML = "Booth 1";
    } 
    else if (x_1>57 && x_1 < 100 &&  y_1 > 50 && y_1 < 100){
      document.getElementById("lokasi").value = "Booth 2";
      document.getElementById("reading-location").innerHTML = "Booth 2";
    }
    else{
      document.getElementById("lokasi").value = "Jalan";
      document.getElementById("reading-location").innerHTML = "Hallway";
    }
    tracker_1.style.position = "absolute";
    tracker_1.style.left = x_11 ;
    tracker_1.style.bottom = y_11 ; 
  });
});


databasex2.on('value', (snapshot) => {
  x2Reading = snapshot.val();
  databasey2.on('value',(snapshot) =>{
    y2Reading=snapshot.val();
    var x_12 = Math.floor(Math.abs(x2Reading / 7 * 100))+"%";
    var y_12 = Math.floor(Math.abs(y2Reading /8 * 100))+"%";
    var x_2 = Math.floor(Math.abs(x2Reading / 7 * 100));
    var y_2 = Math.floor(Math.abs(y2Reading /8 * 100));
    tracker_2.style.position = "absolute";
    tracker_2.style.left = x_12 ;
    tracker_2.style.bottom = y_12 ; 
  });
});
