
var batt = 'device1/battery'
const databasebat = database.ref(batt);
var batteryReading;

//Various Var
var delay = 1000;
var i;
var j;

// var act_1 = document.querySelector(".logo-act-1");
// var act_2 = document.querySelector(".logo-act-2");
// var act_3 = document.querySelector(".logo-act-3");
const map = document.querySelector(".map");

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
