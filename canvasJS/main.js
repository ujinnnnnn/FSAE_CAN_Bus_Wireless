const ws = new WebSocket("ws://192.168.243.20:8000");


function graphNewdata(this_chart,this_dataset,dataAsString){
  var x =  Math.round(((new Date()).getTime() - series_data["epoch"])/100),y = parseFloat(dataAsString)
  series_data[this_dataset].push({
    x : x,
    y: y
  });

  if (series_data[this_dataset].length > 100){
    series_data[this_dataset].shift();
  }
  this_chart.render();
  sessionStorage.setItem("series_data",JSON.stringify(series_data));
}

if (sessionStorage.getItem("series_data")){
  series_data = JSON.parse(sessionStorage.getItem("series_data"));

}else{
  series_data = {
    "epoch" : (new Date()).getTime(),
    "inv_temp" : [],
    "motor_temp" : [],
    "inv_voltage" : [],
    "inv_erpm" : [],
    "inv_duty_cycle" : [],
    "inv_ac_current" : [],
    "inv_dc_current" : [],
    "inv_throttle_in" : [],
    "bms_open_voltage" : [],
    "bms_soc" : [],
    "bms_average_current" : [],
    "bms_high_temperature" : [],
    "bms_internal_temperature" : [],
    "plex_radiator_in" : [],
    "plex_radiator_out" : [],
    "plex_throttle_1" : [],
    "plex_throttle_2" : [],
    "plex_brake" : []
  };

  console.log("no existing series_data");
}
let temperature_chart,hv_voltage_chart;
window.onload = (event) => {
  hv_voltage_chart = new CanvasJS.Chart("hv_voltage_chart",{
    title : {
      text : "hv voltage"
    },
    data : [{
      type : "line",
      dataPoints: series_data["inv_voltage"]
    },
    {
      type : "line",
      dataPoints: series_data["bms_open_voltage"]
    }]
  });


  temperature_chart = new CanvasJS.Chart("temperature_chart",{
    title : {
      text : "Temperature (C)"
    },
    data : [{
      type : "line",
      dataPoints:  series_data["inv_temp"]
    },
    {
      type : "line",
      dataPoints: series_data["motor_temp"] 
    },
    {
      type : "line",
      dataPoints : series_data["bms_internal_temperature"] 

    }]
  });

}

ws.onopen = () => {
    console.log("Connected to the WebSocket server");
};

ws.onmessage = (event) => {
  data = JSON.parse(event.data);
  while (document.readyState != "complete");
  for (const key in data){
    switch (key){
      case "inv_temp":
        graphNewdata(temperature_chart,"inv_temp",data[key]);
        break;
      case "motor_temp":
        graphNewdata(temperature_chart,"motor_temp",data[key]);
        break;
      case "inv_voltage":
        graphNewdata(hv_voltage_chart,"inv_voltage",data[key]);
        break;
      case "inv_foc_id":
        break;
      case "inv_foc_iq":
        break;
      case "inv_erpm":
        break;
      case "inv_duty_cycle":
        break;
      case "inv_ac_current":
        break;
      case "inv_dc_current":
        break;
      case "inv_throttle_in":
        break;
      case "inv_brake_in":
        break;
      case "inv_drive_en":
        break;
      case "inv_can_map_vers":
        break;
      case "inv_digital_io":
        break;
      case "inv_limits_4":
        break;
      case "inv_limits_5":
        break;
      case "inv_fault_code":
        break;
      case "bms_high_open_voltage":
        break;
      case "bms_low_open_voltage":
        break;
      case "bms_high_open_id":
        break;
      case "bms_low_open_id":
        break;
      case "bms_pack_dcl":
        break;
      case "bms_pack_abs_current":
        break;
      case "bms_open_voltage":
        break;
      case "bms_soc":
        break;
      case "bms_pack_current":
        break;
      case "bms_average_current":
        break;
      case "bms_high_temperature":
        break;
      case "bms_internal_temperature":
        graphNewdata(temperature_chart,"bms_internal_temperature",data[key]);
        break;
      case "bms_dtc_flags_1":
        break;
      case "bms_dtc_flags_2":
        break;
      case "bms_balancing_enabled":
        break;
      case "bms_discharge_enable_inverted":
        break;
      case "plex_radiator_in":
        break;
      case "plex_radiator_out":
        break;
      case "plex_battery_voltage":
        break;
      case "plex_gps_fix":
        break;
      case "plex_can1_load":
        break;
      case "plex_can1_errors":
        break;
      case "plex_throttle_1":
        break;
      case "plex_throttle_2":
        break;
      case "plex_brake":
        break;
      case "plex_acc_long":
        break;
      case "plex_acc_lat":
        break;
      case "plex_acc_vert":
        break;
      case "plex_yaw_rate":
        break;
      case "plex_pitch":
        break;
      case "plex_roll":
        break;

      default:
        console.log(key);
    }
  }

};
