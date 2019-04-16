import os
import random
import subprocess
random.seed(30) #first call
for i in range(100):
    x = random.randint(0,100)
    y = random.randint(0,100)
    subprocess.call("cleos push action bingo login \'[" + "careyuyu" + ", " + str(x) + "]\' -p careyuyu@active", shell=True)
    subprocess.call("cleos push action bingo login \'[" + "chip.cat" + ", " + str(y) + "]\' -p chip.cat@active", shell=True)
    os.popen("sleep 1")
    print("1")
    subprocess.call("cleos push action bingo entergame \'[" + "careyuyu" + ", " + str(x) + "]\' -p careyuyu@active", shell=True)
    subprocess.call("cleos push action bingo entergame \'[" + "chip.cat" + ", " + str(y) + "]\' -p chip.cat@active", shell=True)
    os.popen("sleep 1")
    print("2")
    for j in range(5):
        subprocess.call("cleos push action bingo genball \'[" + "careyuyu" + ", " + str(x) + "]\' -p careyuyu@active", shell=True)
        subprocess.call("cleos push action bingo genball \'[" + "chip.cat" + ", " + str(y) + "]\' -p chip.cat@active", shell=True)
        os.popen("sleep 1")
        print("3")
    subprocess.call("cleos push action bingo declearwin \'[" + "careyuyu" + "]\' -p careyuyu@active", shell=True)
    subprocess.call("cleos push action bingo declearwin \'[" + "chip.cat" + "]\' -p chip.cat@active", shell=True)
    os.popen("sleep 1")
    print("4")
    subprocess.call("cleos push action bingo logout \'[" + "careyuyu" + "]\' -p careyuyu@active", shell=True)
    subprocess.call("cleos push action bingo logout \'[" + "chip.cat" + "]\' -p chip.cat@active", shell=True)
    os.popen("sleep 1")
    print("5")