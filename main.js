const ws = new WebSocket("ws://192.168.1.132:8000");

var epochTime = (new Date()).getTime();

function graphNewdata(this_chart,dataAsString){
  var x =  (((new Date()).getTime() - epochTime)/1000).toFixed(2),y = parseFloat(dataAsString)
  if (this_chart.data.datasets[0].data.length > 100) {
    this_chart.data.labels.shift();
      this_chart.data.labels.push(x);
      this_chart.data.datasets[0].data.shift();
      this_chart.data.datasets[0].data.push(y);
  } else {
      this_chart.data.datasets[0].data.push(y);
      this_chart.data.labels.push(x);
  }
  this_chart.update('none');
}


var config = {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      borderColor: "rgb(0, 0, 0)",
      fill : false,
      lineTension : 0,
      data: []
    }]
  },
  options: {
    legend : {display : false},
    title : {
      display : true,
      fontSize : 15
    },
    scales : {
      y : {
        title : {
          display : true,
          text : "y label"
        }
      },
      x : {
        title : {
          display : true,
          text : "x label"
        }
      }
    }
  }
};

let input_voltage_config = JSON.parse(JSON.stringify(config));
config.options.title.text = "DTI Input Voltage";
config.options.scales["y"].title.text = "Voltage (V)";

let input_voltage_chart = new Chart("input_voltage_chart", input_voltage_config);


let motor_temp_config = JSON.parse(JSON.stringify(config));
let motor_temp_chart = new Chart("motor_temp_chart", motor_temp_config);

let inv_temp_config = JSON.parse(JSON.stringify(config));
let inv_temp_chart = new Chart("inv_temp_chart", inv_temp_config);

let average_current_config = JSON.parse(JSON.stringify(config));
let average_current_chart = new Chart("average_current_chart", average_current_config);

let pack_soc_config = JSON.parse(JSON.stringify(config));
let pack_soc_chart = new Chart("pack_soc_chart", pack_soc_config);

let bms_internal_temp_config = JSON.parse(JSON.stringify(config));
let bms_internal_temp_chart = new Chart("bms_internal_temp_chart", bms_internal_temp_config);



function sendMessage() {
    const input = document.getElementById("message");
    const message = `${input.value}`;
    ws.send(message);
    input.value = "";
}
ws.onopen = () => {
    console.log("Connected to the WebSocket server");
};





ws.onmessage = (event) => {
  data = JSON.parse(event.data);
  for (const key in data){
    switch (key){
      case "inv-temp":
        graphNewdata(inv_temp_chart,data[key]);
        break;
      case "motor-temp":
        graphNewdata(motor_temp_chart,data[key]);
        break;
      case "average-current":
        graphNewdata(average_current_chart,data[key]);
        break;
      case "pack-soc":
        graphNewdata(pack_soc_chart,data[key]);
        break;
      case "bms-internal-temp":
        graphNewdata(bms_internal_temp_chart,data[key]);
        break;
      case "input-voltage":
        graphNewdata(input_voltage_chart,data[key]);
        break;
      case "pack-abs-current":
        document.getElementById("pack_abs_current").innerHTML = parseInt(data[key]) + "A";
        break;
      case "inv-fault-code":
        document.getElementById("fault_code").innerHTML = "0x" + parseInt(data[key]).toString(16).toUpperCase();
        break;
      case "bms-high-temp":
        break;
      case "max-cell-voltage":
        break;
      case "max-pack-voltage":
        break;

      default:
        console.log(key);
    }
  }

};
