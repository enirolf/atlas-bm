#! /usr/bin/env bash

# convert figures/readspeed_HDD_mc.png -transparent white -fuzz 90% figures/readspeed_HDD_mc.png

for img in {readspeed,readrate}_overview_{SSD,HDD}_{data,mc}; do
  convert figures/${img}.png -transparent white -fuzz 90% figures/${img}_transp.png
done
