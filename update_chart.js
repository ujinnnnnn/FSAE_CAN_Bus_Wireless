const ws = new WebSocket("ws://192.168.243.20:8000");

input_voltage_chart = new Chart("input_voltage_chart", {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      backgroundColor:"rgba(0,0,255,1.0)",
      borderColor: "rgba(0,0,255,0.1)",
      data: []
    }]
  },
});

motor_temp_chart = new Chart("motor_temp_chart", {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      backgroundColor:"rgba(0,0,255,1.0)",
      borderColor: "rgba(0,0,255,0.1)",
      data: []
    }]
  },
});

inv_temp_chart = new Chart("inv_temp_chart", {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      backgroundColor:"rgba(0,0,255,1.0)",
      borderColor: "rgba(0,0,255,0.1)",
      data: []
    }]
  },
});

average_current_chart = new Chart("average_current_chart", {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      backgroundColor:"rgba(0,0,255,1.0)",
      borderColor: "rgba(0,0,255,0.1)",
      data: []
    }]
  },
});


pack_soc_chart = new Chart("pack_soc_chart", {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      backgroundColor:"rgba(0,0,255,1.0)",
      borderColor: "rgba(0,0,255,0.1)",
      data: []
    }]
  },
});

bms_temperature_chart = new Chart("bms_temperature_chart", {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      backgroundColor:"rgba(0,0,255,1.0)",
      borderColor: "rgba(0,0,255,0.1)",
      data: []
    }]
  },
});



function sendMessage() {
    const input = document.getElementById("message");
    const message = `${input.value}`;
    ws.send(message);
    input.value = "";
}
ws.onopen = () => {
    console.log("Connected to the WebSocket server");
};



function graphNewdata(this_chart,dataAsString){
  var x = (new Date()).getTime(),y = parseFloat(dataAsString)
  if (this_chart.data.datasets[0].data.length > 10) {
      this_chart.data.labels.pop();
      this_chart.data.labels.push(x);
      this_chart.data.datasets[0].data.pop();
      this_chart.data.datasets[0].data.push(y);
  } else {
      this_chart.data.datasets[0].data.push(y);
      this_chart.data.labels.push(x);
  }
  this_chart.update();
}



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
      case "max-cell-voltage":
        break;
      case "input-voltage":
        graphNewdata(input_voltage_chart,data[key]);
        break;
      case "pack-abs-current":
        document.getElementById("pack_abs_current").innerHTML = parseInt(data[key]) + "A";
        break;
      case "average-current":
        graphNewdata(average_current_chart,data[key]);
        break;
      case "max-pack-voltage":
        break;
      case "inv-fault-code":
        document.getElementById("fault_code").innerHTML = "0x" + parseInt(data[key]).toString(16).toUpperCase();
        break;
      case "pack-soc":
        graphNewdata(pack_soc_chart,data[key]);
        break;
      case "bms-high-temp":
        break;
      case "bms-internal-temp":
        graphNewdata(bms_temperature_chart,data[key]);
        break;

      default:
        console.log(key);
    }
  }

};
