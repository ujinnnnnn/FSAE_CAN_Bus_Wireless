for (let i = 0; i < 120; i++) {
  document.getElementById("CELL_".concat(i.toString())).innerHTML = (3.68 + 0.05*Math.random()).toString(10);
}