function getRandomColor() {
    return `rgb(${Math.floor(Math.random() * 255)}, ${Math.floor(Math.random() * 255)}, ${Math.floor(Math.random() * 255)})`
  }
  function changeColor() {
      var color = getRandomColor();
      if ($("#square1").css("color") != color) {
        $(this).css("background-color", color);
        console.log(this.id)
      } else {
        $(this).css("background-color", getRandomColor());
      }
    $(document).ready(function(){
        $(".balls").click(function(){
          $(this).css("background-color", getRandomColor());
        });
      });
  }