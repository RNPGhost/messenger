// Get the current time as a string of the form "YYYY-MM-DD HH:MM"
function now() {
  var now = new Date;
  var year = now.getFullYear(),
      month = now.getMonth() + 1,
      day = now.getDate(),
      hour = now.getHours(),
      minute = now.getMinutes();
  var pad = (x, width) => x.toString().padStart(width, "0");
  var date = pad(year, 4) + "-" + pad(month, 2) + "-" + pad(day, 2);
  var time = pad(hour, 2) + ":" + pad(minute, 2);
  return date + " " + time;
}

// Send a message containing the given text.
function sendMessage(htmlString) {
  // Fake it. Just show it locally lolz.
  var element = document.createElement("div");
  element.className = "sent";
  element.setAttribute("data-timestamp", now());
  element.innerHTML = htmlString;
  content.appendChild(element);
  content.scrollTop = content.scrollHeight;

  var query = new XMLHttpRequest();
  query.open("POST", "/message")
  query.send(htmlString);
}

// Pick up the enter key being pressed in the message box, send the message,
// clear the box.
message.addEventListener("keydown", event => {
  if (event.key != "Enter") return;
  sendMessage(message.innerHTML);
  while (message.firstChild) message.removeChild(message.firstChild);
  event.preventDefault();
});
