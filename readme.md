- [Code organization](#code-organization)
- [Command examples](#command-examples)

This repo consists a simple blockchain-based application that allows user to play an online provably fair bingo.

> Guohui Li, Fangzheng Zhang, Yuqing Liu 

The original code for the paper belongs to Google, and there seems to be no need to go through all the legal processes to open source them as the experiments are rather straightforward to implement and reproduce.

However, since people have been asking for the code. In order for people to easily get started, I created this repo to demonstrate how one can take an existing implementation of commonly used successful models (here we use [an implementation of Wide Resnets in pytorch](https://github.com/xternalz/WideResNet-pytorch)) and fit them to random labels. We are not trying to reproduce exactly the same experiments (e.g. with Inception and Alexnet and corresponding hyper parameters) in the paper. This repo is completely written from scratch, and has no association with Google.

If you are trying to play the game, you can access this following link:

>

- Sign up account via the website UI.
- Login the game and wait for another player.
- Enter the game and the game will automatically started.
- It will assign a 5x5 grid with numbers, and generate first balls for matching.
- It will generate 1 ball every turn in following five turns.
- The user can declare his/her victory via the website UI if they think they are winning. Otherwise, they will logout from the game.

# Code organization

- `bingo`: the folder the contains bingo.cpp, bingo.wasm, and bingo.abi.
- `Table&balls`: the folder that contains the front-end portion and the networking and communication between front-end and back-end.
- `Google_Cloud_EOS_CMD`: the folder that contains the user instructon for testing the smart contract function in the terminal of the server, includes the test script and command instruction.
- 

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

# Compilated Instruction
