function ausgabe(wert, zahl){
		var absmin;
		var absmax;
		var okmin;
		var okmax;
		switch(wert){ 
			case "ph":
				absmin=7.0;
				okmin=7.2;
				okmax=7.4;
				absmax=7.6;
			break;
			case "tds": case "ec":
				absmin=500;
				okmin=1000;
				okmax=2000;
				absmax=3000;
			break;
			case "orp":
				absmin=0.6;
				okmin=0.65;
				okmax=0.75;
				absmax=0.8;
			break;
			case "battery":
				absmin=10;
				okmin=10;
				okmax=100;
				absmax=110;
			break;
			case "cl":
				absmin=0.3;
				okmin=0.5;
				okmax=1.0;
				absmax=3.0;
			break;
		}
		let farbe;
		if ( zahl < absmin ) { farbe="blue" } else
		if ( zahl > absmax) { farbe="red" } else
		if ( zahl >= okmin && zahl <= okmax ) { farbe="green" } else
		{ farbe="orange" };
		if ( wert == "salt" || wert == "temp") { farbe="black" };
		document.getElementById(wert).innerHTML = "<font color=" + farbe + ">" + zahl + "</font>";
}
async function getData() {
  const url = "json";
  try {
    const response = await fetch(url);
    if (!response.ok) {
      throw new Error(`Response status: ${response.status}`);
    }
    const json = await response.json();
    const utc = new Date().toUTCString();
    document.getElementById("zeit").innerHTML = "(" + utc + ")";
	ausgabe("ph", json.ph);
	ausgabe("cl", json.cl);
	ausgabe("tds", json.tds);
	ausgabe("ec", json.ec);
	ausgabe("orp", json.orp);
	ausgabe("salt", json.salt);
	ausgabe("temp", json.temp);
	ausgabe("battery", json.battery);
} catch (error) {
    console.error(error.message);
  }
}

getData();
const sekunden=1000;
var blubb = setInterval(getData, 20*sekunden);
