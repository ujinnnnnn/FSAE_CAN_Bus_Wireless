const ws = new WebSocket("ws://192.168.1.132:8000");


var epochTime = (new Date()).getTime();

function graphNewdata(this_chart,dataAsString){
  console.log(this_chart)
  var x =  ((new Date()).getTime() - epochTime)/1000,y = parseFloat(dataAsString)
  if (this_chart.data.datasets[0].data.length > 10) {
    this_chart.data.labels.shift();
      this_chart.data.labels.push(x);
      this_chart.data.datasets[0].data.shift();
      this_chart.data.datasets[0].data.push(y);
  } else {
      this_chart.data.datasets[0].data.push(y);
      this_chart.data.labels.push(x);
  }
}

input_voltage_chart = Plotly.newPlot("input_voltage_chart", 
  {
    x : [],
    y : [],
    mode : 'lines+markers',
    name : 'Input Voltage'
  }, 
  {
    title : {
      text: "DTI Input Voltage"
    },
    xaxis : {
      title :{
        text : "time (s)"
      }
    },
    yaxis : {
      title : {
        text : "Voltage (V)"
      }
    }
  });

ws.onopen = () => {
    console.log("Connected to the WebSocket server");
};

ws.onclose = () => {
  console.log("bye");
};




ws.onmessage = (event) => {
  data = JSON.parse(event.data);
  for (const key in data){
    switch (key){
      case "inv-temp":
        //graphNewdata(inv_temp_chart,data[key]);
        break;
      case "motor-temp":
        //graphNewdata(motor_temp_chart,data[key]);
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
        //graphNewdata(average_current_chart,data[key]);
        break;
      case "max-pack-voltage":
        break;
      case "inv-fault-code":
        document.getElementById("fault_code").innerHTML = "0x" + parseInt(data[key]).toString(16).toUpperCase();
        break;
      case "pack-soc":
        //graphNewdata(pack_soc_chart,data[key]);
        break;
      case "bms-high-temp":
        break;
      case "bms-internal-temp":
        //graphNewdata(bms_internal_temp_chart,data[key]);
        break;

      default:
        console.log(key);
    }
  }

};
