/**
 * Copyright 2017, Google, Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

'use strict';

// [START gae_flex_quickstart]
const express = require('express'); 
const apirouter = express();
const pug = require('pug');
const pug_index = pug.compileFile('index.pug');
const pug_gameEntry = pug.compileFile('gameEntry.pug');
const pug_gamePage = pug.compileFile('gamePage.pug');
var bodyParser = require('body-parser');
var session = require('express-session');
apirouter.use(bodyParser());
apirouter.use(session({secret:"bingo",uid:"",seed:""}));
const { Api, JsonRpc, RpcError } = require('eosjs');
const { JsSignatureProvider } = require('eosjs/dist/eosjs-jssig');      // development only
const { TextEncoder, TextDecoder } = require('util');                   // node only; native TextEncoder/Decoder
const fetch = require('node-fetch');  
const defaultPrivateKey = "5Jkf26teTgUUm5BSD6g6CTHKnAF6oidquCd6AAoZaNugPzev6yq"; 
const signatureProvider = new JsSignatureProvider([defaultPrivateKey]);

const rpc = new JsonRpc('http://localhost:8888', { fetch });
const api = new Api({ rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() });



apirouter.get('/api', (req, res) => {
  res
    .status(200).json({status: 'OK'});
    //.sendFile('index.html', {root: './'});
});

apirouter.post('/login', (req, res) => {
	const { exec } = require('child_process');
	exec(`cleos get account ${req.body.uid} --json`, (error, stdout, stderr) => {
	  if (error) {
		  console.log(error);
		  return res.redirect('http://35.199.5.180');
	  }
	  else {
		  req.session.uid = req.body.uid;
			res.end(pug_index({
				uid: req.session.uid,
				logined: true
		}));
	  }
	});
});

apirouter.post('/logout', (req, res) => {
	req.session.uid = "";
	res.end(pug_index({
		uid: req.session.uid,
		logined: false
	}));
});

apirouter.post('/', (req, res) => {
	var login = true
	if (req.session.uid==""){
		login = false
	}
	res.end(pug_index({
		uid: req.session.uid,
		logined: login
	}));
});

apirouter.post('/logingame', (req, res) => {
	(async () => {
	  try {
		  const result = await api.transact({
			actions: [{
			  account: 'bingo',
			  name: 'login',
			  authorization: [{
				actor: `${req.session.uid}`,
				permission: 'active',
			  }],
			  data: {
				username: `${req.session.uid}`,
				seed: `${req.body.seed}`,
			  },
			}]
		  }, {
			blocksBehind: 3,
			expireSeconds: 30,
		  });
		  console.dir(result);
		  req.session.seed = req.body.seed;
		} catch (e) {
		  console.log('\nCaught exception: ' + e);
		  if (e instanceof RpcError)
			console.log(JSON.stringify(e.json, null, 2));
		}
	})();
	getUserByName(`${req.session.uid}`).then(val => {
		res.end(pug_gameEntry({
			uid: req.session.uid,
			logined: true,
			win: val.win_count,
			lost: val.lost_count,
			seed1: val.seed
		}));
	});
	//console.log(playerInfo)
});

apirouter.post('/declareWin', (req, res) => {
	(async () => {
	  try {
		  const result = await api.transact({
			actions: [{
			  account: 'bingo',
			  name: 'declarewin',
			  authorization: [{
				actor: `${req.session.uid}`,
				permission: 'active',
			  }],
			  data: {
				username: `${req.session.uid}`,
			  },
			}]
		  }, {
			blocksBehind: 3,
			expireSeconds: 30,
		  });
		  console.dir(result);
		} catch (e) {
		  console.log('\nCaught exception: ' + e);
		  if (e instanceof RpcError)
			console.log(JSON.stringify(e.json, null, 2));
		}	
	})();
	getUserBoard(`${req.session.uid}`).then(val => {
		var balls = [];
		var index = 0;
		while(val.balls[index] != -1 && index < 10) {
			balls[index] = val.balls[index];
			index++;
		}
		res.end(pug_gamePage({
			prev: val.prev_game,
			ingame: val.in_game,
			board: val.board,
			balls: balls
		}));
	});
	//console.log(playerInfo)
});

apirouter.post('/entergame', (req, res) => {
	entergame(req.session.uid, req.session.seed)
	getUserBoard(`${req.session.uid}`).then(val => {
		var balls = [];
		var index = 0;
		while(val.balls[index] != -1 && index < 10) {
			balls[index] = val.balls[index];
			index++;
		}
		res.end(pug_gamePage({
			prev: val.prev_game,
			ingame: 1,
			board: val.board,
			balls: balls
		}));
		
	});
});

apirouter.post('/genball', (req, res) => {
	(async () => {
	  try {
		  const result = await api.transact({
			actions: [{
			  account: 'bingo',
			  name: 'genball',
			  authorization: [{
				actor: `${req.session.uid}`,
				permission: 'active',
			  }],
			  data: {
				username: `${req.session.uid}`,
				b_seed: `${req.session.seed}`,
			  },
			}]
		  }, {
			blocksBehind: 3,
			expireSeconds: 30,
		  });
		  console.dir(result);
		} catch (e) {
		  console.log('\nCaught exception: ' + e);
		  if (e instanceof RpcError)
			console.log(JSON.stringify(e.json, null, 2));
		}	
	})();
	getUserBoard(`${req.session.uid}`).then(val => {
		var balls = [];
		var index = 0;
		while(val.balls[index] != -1 && index < 10) {
			balls[index] = val.balls[index];
			index++;
		}
		res.end(pug_gamePage({
			prev: val.prev_game,
			ingame: val.in_game,
			board: val.board,
			balls: balls
		}));
		
	});
});

async function entergame(uid, seed) {
  try {
	  const result = await api.transact({
		actions: [{
		  account: 'bingo',
		  name: 'entergame',
		  authorization: [{
			actor: uid,
			permission: 'active',
		  }],
		  data: {
			username: uid,
			b_seed: seed,
		  },
		}]
	  }, {
		blocksBehind: 3,
		expireSeconds: 30,
	  });
	  console.dir(result);
	} catch (e) {
		if (e != "Error: assertion failure with message: Player already in game!") {
			console.log('\nCaught exception: ' + e);
			if (e instanceof RpcError)
				console.log(JSON.stringify(e.json, null, 2));
			await sleep(1000);
			entergame(uid, seed);
		}
	}	
}

async function getUserBoard(username) {
	//console.log("aaaaaaaaaaa" + username)
	var toReturn = undefined
	while (1) {
		await sleep(1000);
		//console.log("1");
		try {
		  const result = await rpc.get_table_rows({
			"json": true,
			"code": "bingo",    // contract who owns the table
			"scope": "bingo",   // scope of the table
			"table": "players",    // name of the table as specified by the contract abi
			"limit": 1,
			"lower_bound": username,
		  });
		  if (result == undefined) {
			  //console.log("1");
			  continue;
		  }
		  toReturn = result.rows[0].board[0];
		  if (toReturn != -1) {
			  toReturn = result.rows[0];
			  break;
		  }
		  //return result.rows[0];
		} catch (err) {
		  console.error(err);
		}
	}
	return toReturn
}


async function getUserByName(username) {
	var toReturn = undefined
	while (toReturn == undefined) {
		try {
		  const result = await rpc.get_table_rows({
			"json": true,
			"code": "bingo",    // contract who owns the table
			"scope": "bingo",   // scope of the table
			"table": "players",    // name of the table as specified by the contract abi
			"limit": 1,
			"lower_bound": username,
		  });
		  toReturn = result.rows[0];
		  //return result.rows[0];
		} catch (err) {
		  console.error(err);
		}
	}
	return toReturn
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}


const app = express();
app.use(apirouter);
app.use('/', express.static(__dirname + '/public'));




// Start the server
const PORT = process.env.PORT || 8080;

app.listen(PORT, () => {
  console.log(`App listening on port ${PORT}`);
  console.log('Press Ctrl+C to quit.');
});
// [END gae_flex_quickstart]
