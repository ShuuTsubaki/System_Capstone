- [Code organization](#code-organization)
- [Contract Command Examples](#contract-command-examples)
- [Set Up Instruction](#set-up-instruction)

This repo consists a simple EOISO-based application that allows user to play an online provably fair bingo.

> Guohui Li, Fangzheng Zhang, Yuqing Liu, Hongwei Lou, Zeke Lin 


If you are trying to play the game, you can access this following link:
```
http://35.199.5.180/
```
>

- Login the game with EOSIO account.
- Input the see and wait for another player.
- Enter the game and the game will automatically started.
- It will assign a 5x5 grid with numbers, and generate first balls for matching.
- It will generate 1 ball every turn in following five turns.
- The user can declare his/her victory via the website UI if they think they are winning. Otherwise, they will logout from the game.

# Code organization

- `bingo`: the folder the contains bingo.cpp, bingo.wasm, and bingo.abi.
- `Table&balls`: the folder that contains the front-end portion and the networking and communication between front-end and back-end.
- `Google_Cloud_EOS_CMD`: the folder that contains the user instructon for testing the smart contract function in the terminal of the server, includes the test script and command instruction.
- `Server_Folder`: the folder contains all the server files. All the files in this folder are ready to run. (the commands to start this server are in "Set Up Instruction" part.)

## File inside the bingo folder

- `bingo.cpp`: the smart contract of this application.
- `bingo.wasm`: the web assembly version of smart contract.
- `bing.abi`: the JSON-based description on how the convert the user action in bingo.cpp between their JSON and Binary representation. It allows the user to interact with the contract via JSON.

# Contract Command Examples 

```
$User_Name: careyuyu
```

Login: 
```
$User_Name = the username
$Seed = User's seed
cleos push action bingo login '["$User_Name", "$Seed"]' -p $User_Name@active
```
Enter game：
```
cleos push action bingo entergame '["$User_Name"]' -p $User_Name@active
```
Genball：
```
cleos push action bingo genball '["$User_Name"]' -p $User_Name@active
```
Declearwin：
```
cleos push action bingo declarewin '["$User_Name"]' -p $User_Name@active
```
Logout:
```
cleos push action bingo logout '["$User_Name"]' -p $User_Name@active
```
Erase table：
```
cleos push action bingo erase '['bingo']' -p bingo@active
```
Get Table:
```
cleos get table bingo bingo players
```

# Set Up Instruction
`Before You Start`: 
- It may not work correctly in your own computer, you need change our server address to your own address
- Make sure you download nodejs and npm
- Make sure you download the Server_Folder, and make sure you are in it. 

Setup:
```
npm install
```
Start Server:
```
sudo PORT=80 npm start
```

You are ready to go! You can connect to your localhost to play the game.
