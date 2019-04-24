function login() {
	//alert(document.getElementById("uid").value)
	var response = "no response"
	const { exec } = require('child_process');
	exec('cleos get account bingo --json', (error, stdout, stderr) => {
	  if (error) {
		console.error(`exec error: ${error}`);
		return;
	  }
	  response = JSON.parse(stdout);
	  //console.log(`stdout: ${stdout}`);
	  //console.log(`stderr: ${stderr}`);
	});
	alert(response)
}