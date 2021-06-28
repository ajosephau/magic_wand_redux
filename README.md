# magic_wand_redux
 
## Processing output from serial

split -p timestamp,accX,accY,accZ up-and-down.csv up-down-
for i in up-down-*; do mv "$i" "$i.csv"; done