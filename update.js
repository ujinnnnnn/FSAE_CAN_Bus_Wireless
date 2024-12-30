const ws = new WebSocket("ws://192.168.243.20:8000");


function sendMessage() {
    const input = document.getElementById("message");
    const message = `${input.value}`;
    ws.send(message);
    input.value = "";
}
ws.onopen = () => {
    console.log("Connected to the WebSocket server");
};



function graphNewdata(chart,dataAsString){
  var x = (new Date()).getTime(),y = parseFloat(dataAsString)
  if (chart.series[0].data.length > 10) {
      chart.series[0].addPoint([x, y], true, true, true);
  } else {
      chart.series[0].addPoint([x, y], true, false, true);
  }
}
ws.onmessage = (event) => {
  console.log(event)
  data = JSON.parse(event.data);
  for (const key in data){
    switch (key){
      case "inv-temp":
        graphNewdata(chartInvTemp,data[key]);
        break;
      case "motor-temp":
        graphNewdata(chartMotorTemp,data[key]);
        break;
      case "max-cell-voltage":
        break;
      case "input-voltage":
        graphNewdata(chartInvVoltage,data[key]);
        break;
      case "pack-abs-current":
        document.getElementById("pack_abs_current").innerHTML = parseInt(data[key]) + "A";
        break;
      case "average-current":
        graphNewdata(chartAverageCurrent,data[key]);
        break;
      case "max-pack-voltage":
        break;
      case "inv-fault-code":
        console.log(data[key])
        document.getElementById("fault_code").innerHTML = "0x" + parseInt(data[key]).toString(16).toUpperCase();
        break;
      case "pack-soc":
        graphNewdata(chartPackSoc,data[key]);
        break;
      case "bms-high-temp":
        break;
      case "bms-internal-temp":
        graphNewdata(chartBmsTemp,data[key]);
        break;

      default:
        console.log(key);
    }
  }

};


var chart_update_period = 1000;

var chartInvVoltage = new Highcharts.Chart({
  chart: { renderTo: 'chart-input-voltage' },
  title: { text: 'Inverter Input Voltage' },
  series: [{
      showInLegend: false,
      data: []
  }],
  plotOptions: {
      line: {
          animation: false,
          dataLabels: { enabled: true }
      },
      series: { color: '#059e8a' }
  },
  xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
      title: { text: 'Input Voltage (V)'}
  },
  credits: { enabled: false }
});

var chartMotorTemp = new Highcharts.Chart({
  chart: { renderTo: 'chart-motor-temp' },
  title: { text: 'Motor Temperature' },
  series: [{
      showInLegend: false,
      data: []
  }],
  plotOptions: {
      line: {
          animation: false,
          dataLabels: { enabled: true }
      }
  },
  xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
      title: { text: 'Motor Temperature (C)' }
  },
  credits: { enabled: false }
});

var chartInvTemp = new Highcharts.Chart({
  chart: { renderTo: 'chart-inv-temp' },
  title: { text: 'Inverter Temperature' },
  series: [{
      showInLegend: false,
      data: []
  }],
  plotOptions: {
      line: {
          animation: false,
          dataLabels: { enabled: true }
      },
      series: { color: '#18009c' }
  },
  xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
      title: { text: 'Inverter Temperature (C)' }
  },
  credits: { enabled: false }
});

var chartAverageCurrent = new Highcharts.Chart({
  chart: { renderTo: 'chart-average-current' },
  title: { text: 'BMS Average Current' },
  series: [{
      showInLegend: false,
      data: []
  }],
  plotOptions: {
      line: {
          animation: false,
          dataLabels: { enabled: true }
      },
      series: { color: '#18009c' }
  },
  xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
      title: { text: 'BMS Average Current (A)' }
  },
  credits: { enabled: false }
});

var chartPackSoc = new Highcharts.Chart({
    chart: { renderTo: 'chart-pack-soc' },
    title: { text: 'BMS Pack SoC (%)' },
    series: [{
        showInLegend: false,
        data: []
    }],
    plotOptions: {
        line: {
            animation: false,
            dataLabels: { enabled: true }
        },
        series: { color: '#18009c' }
    },
    xAxis: {
        type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
        title: { text: 'BMS Pack SoC (%)' }
    },
    credits: { enabled: false }
});

var chartBmsTemp = new Highcharts.Chart({
    chart: { renderTo: 'chart-bms-temperature' },
    title: { text: 'BMS Internal Temperature' },
    series: [{
        showInLegend: false,
        data: []
    }],
    plotOptions: {
        line: {
            animation: false,
            dataLabels: { enabled: true }
        },
        series: { color: '#18009c' }
    },
    xAxis: {
        type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
        title: { text: 'BMS Internal Temperature (C)' }
    },
    credits: { enabled: false }
});