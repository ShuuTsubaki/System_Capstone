function numberGenerator() {
  var n = new Array(25);
  var nums = new Array(25);
  for (let i = 0; i < nums.length; i++) {
    var randomNumber = Math.floor(Math.random() * 70);
    for (let j = 0; j < n.length; j++) {
      if (n[j] == randomNumber)
        randomNumber = Math.floor(Math.random() * 70);
    }
    n[i] = randomNumber;
  }
  // Output to the table
  for (let k = 0; k < n.length; k++) {
    // document.getElementById("square" + k).style.fontsize = "100px";
    document.getElementById("square" + k).innerText = (n[k]);
    document.getElementById("square" + k).style.position = "relative";
    document.getElementById("square" + k).style.top = "30%"
    document.getElementById("square" + k).align = "center";
  }
  document.getElementById("square12").innerHTML = "";
  document.getElementById("square12").style = "none";
  
  // document.getElementById("num1").innerText = (33);
  // document.getElementById("num1").style.position = "relative";
  // document.getElementById("num1").style.top = "30%"
  // document.getElementById("num1").align = "center";
}
function putNumber(arr) {
  for (let k = 0; k < arr.length; k++) {
    // document.getElementById("square" + k).style.fontsize = "100px";
    document.getElementById("square" + k).innerText = (arr[k]);
    document.getElementById("square" + k).style.position = "relative";
    document.getElementById("square" + k).style.top = "30%"
    document.getElementById("square" + k).align = "center";
  }
  document.getElementById("square12").innerHTML = "";
  document.getElementById("square12").style = "none";
}