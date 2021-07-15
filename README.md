# magic_wand_redux
 
This is a magic wand that lets one "cast a spell" by following a particular gesture. I programmed a circuit playground bluefruit with Arduino, Edge Impulse and Tensorflow Lite to recognise three spells.
 
## Processing output from serial

split -p timestamp,accX,accY,accZ up-and-down.csv up-down-
for i in up-down-*; do mv "$i" "$i.csv"; done
