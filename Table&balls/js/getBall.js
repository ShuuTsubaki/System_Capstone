function getNum(numberOnBall) {
    let countBall = 1;
    document.getElementById("num"+ countBall).innerText = numberOnBall;
    document.getElementById("num" + countBall).style.position = "relative";
    document.getElementById("num" + countBall).style.top = "30%"
    document.getElementById("num" + countBall).align = "center";
    countBall++;
}